#define GLEW_STATIC
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>
#ifdef __unix__   
#include <libgen.h>
#include <linux/limits.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

int get(std::ifstream &input);
void window_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void draw(GLFWwindow *window);
std::string get_filename(std::string filePath, char seperator = '\\');
int resize_mode = 0;	// Resize Mode: 0 - Keep proportions; 1 - Fill window
double image_aspect_ratio;	// Height / Width

int main(int argc, char **argv) {
	GLFWwindow *window;

	if (!glfwInit()) {
		std::cerr << "Could not initialise GLFW!\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}

	std::ifstream file;
	file.open(argv[1], std::ios::in | std::ios::binary);
	if (!file) {
		std::cerr << "No such file!\n";
		return EXIT_FAILURE;
	}

	std::string line;
	int size_x;
	int size_y;
	short max_size;
	unsigned char *image;
	GLenum datatype;
	int channels;
	GLenum type;

	// Determine file type
	file >> line;
	if (line == "P5") {
		channels = 1;
		type = GL_LUMINANCE;
	} else if (line == "P6") {
		channels = 3;
		type = GL_RGB;
	} else {
		std::cerr << "Not a supported file type!\n";
		return EXIT_FAILURE;
	}

	// Determine aspect ratio
	size_x = get(file);
	size_y = get(file);
	image_aspect_ratio = (double) size_y / (double) size_x;
	max_size = get(file);
	file.get();
	int rowlength = size_x * channels;

	// Read file
	if (max_size < 256) {
		image = new unsigned char[size_y * rowlength];
		file.read((char *)image, size_y * rowlength);

		if (max_size != 255)
			for (int i = 0; i < size_y; i++)
				for (int j = 0; j < rowlength; j++) {
					// For images with lower dynamic range, increase to 1 byte per channel
					image[i * rowlength + j] = (unsigned char)(((int)image[i * rowlength + j] * 255) / max_size);
				}
		datatype = GL_UNSIGNED_BYTE;
	} else {
		// Not the cleanest sulution (UB!) but it works
		union {
			unsigned short *image;
			unsigned char *image_chars;
		} imgunion;

		imgunion.image = new unsigned short[size_y * rowlength];
		file.read((char *)imgunion.image_chars, sizeof(short) * size_y * rowlength);
		if (max_size != 65535)
			for (int i = 0; i < size_y; i++)
				for (int j = 0; j < rowlength; j++) {
					imgunion.image[i * rowlength + j] = (unsigned short)(((int)imgunion.image[i * rowlength + j] * 65535) / max_size);
				}
		image = imgunion.image_chars;
		datatype = GL_UNSIGNED_SHORT;
	}
	file.close();

	// Init glfw
	window = glfwCreateWindow(size_x, size_y, (get_filename(argv[1], '\\') + " - PGM, PPM Viewer").c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, width, height);
	if (!window) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Init OpenGL
	GLuint imgBuffer;
	glGenTextures(1, &imgBuffer);
	glBindTexture(GL_TEXTURE_2D, imgBuffer);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size_x, size_y, 0, type, datatype, image);

	draw(window);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		usleep(40000);
	}
	delete image;

	return 0;
}

int get(std::ifstream &input) {
	std::string line;
	while (true) {
		char c = input.get();
		if (c != '#' && c != '\n' && c != ' ') {
			std::string lline;
			input >> lline;
			line = c + lline;
			break;
		} else if (c == '#')
			std::getline(input, line);
	}
	return std::stoi(line);
}

std::string get_filename(std::string file_path, char seperator) {
	size_t dot_pos = file_path.rfind('.');
	size_t sep_pos = file_path.rfind(seperator);
	if (sep_pos != std::string::npos)
		return file_path.substr(sep_pos + 1, file_path.size() - (dot_pos != std::string::npos ? 1 : dot_pos));
	return file_path;
}

void window_size_callback(GLFWwindow *window, int width, int height) {
	int width_, height_;
	glfwGetFramebufferSize(window, &width_, &height_);
	glViewport(0, 0, width_, height_);
	draw(window);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_G && action == GLFW_PRESS){
		resize_mode ^= true;	// Flip value
		draw(window);
	}
}

void draw(GLFWwindow *window) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	double x_offset = 1.0;
	double y_offset = 1.0;
	glBegin(GL_QUADS);
	if (!resize_mode) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		double size_ratio = (double) height / (double) width;
		if (size_ratio > image_aspect_ratio)
			y_offset = image_aspect_ratio/size_ratio;
		else
			x_offset = size_ratio/image_aspect_ratio;
	}
	glTexCoord2d(0.0, 0.0);
	glVertex2d(-x_offset, y_offset);

	glTexCoord2d(0.0, 1.0);
	glVertex2d(-x_offset, -y_offset);

	glTexCoord2d(1.0, 1.0);
	glVertex2d(x_offset, -y_offset);

	glTexCoord2d(1.0, 0.0);
	glVertex2d(x_offset, y_offset);
	glEnd();
	glfwSwapBuffers(window);
}
