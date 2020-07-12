#define APP_NAME "SFML-RT"

#include <thread>				// std::thread - concurrency
#include <vector>				// needed for array of threads
#include <float.h>

#include "CLI11.hpp"
#include <SFML/Graphics.hpp> 	// SFML is 5 modules: system, window, graphics, network and audio; don't forget to include them in g++ -g ... -lsfml-window ... etc.
#include <fmt/format.h>			// somewhy won't compile under mingw with linker flag -lfmt; so use in header-only mode with -DFMT_HEADER_ONLY
#include <fmt/ostream.h>		// needed for serialization of std::thread::thread_id via overloaded <<

#include "main.h"

static int window_width = 1280;
static int window_height = 720;
static int ratio_GCD = std::__gcd(window_height, window_width);
static bool window_fullscreen = false;
static int framerate_limiter = 120;

static int amount_of_threads = std::thread::hardware_concurrency() - 1;
std::vector<std::thread> threads(amount_of_threads);

int main(int argc, char *argv[]) {

	CLI::App app(APP_NAME);

	app.add_option("-h,--height", window_height, "The height of the output picture");
	app.add_option("-w,--width", window_width, "The width of the output picture");
	app.add_flag("-f,--fullscreen", window_fullscreen, "Force fullscreen");

	CLI11_PARSE(app, argc, argv);

	sf::VideoMode screenSize = sf::VideoMode::getDesktopMode();
		window_width = window_fullscreen?screenSize.width:window_width;
		window_height = window_fullscreen?screenSize.height:window_height;
		screenSize = sf::VideoMode(window_width, window_height, screenSize.bitsPerPixel);

	sf::Uint32 window_style = (window_fullscreen ? sf::Style::Fullscreen : sf::Style::Resize | sf::Style::Close);

	//GL settings
	sf::ContextSettings GLsettings;
		GLsettings.depthBits = 24;
		GLsettings.stencilBits = 8;
		GLsettings.antialiasingLevel = 4;
		GLsettings.majorVersion = 2;
		GLsettings.minorVersion = 0;

	//Create window
	sf::RenderWindow window;
		window.create(screenSize, APP_NAME, window_style, GLsettings);
		window.setFramerateLimit(framerate_limiter);
		window.requestFocus();

	sf::Vector2u window_size = window.getSize();

	sf::Texture texture;
		if (!texture.create(window_width, window_height)) return EXIT_FAILURE;
	sf::Sprite sprite(texture);

	sf::Uint8* image = new sf::Uint8[window_height * window_width * 4];

	sf::Font font1, font2;
		if (!font1.loadFromFile("./Fonts/CG.ttf")) return EXIT_FAILURE;
		if (!font2.loadFromFile("./Fonts/L.ttf")) return EXIT_FAILURE;

	sf::Text text;
		text.setFont(font1);
		text.setCharacterSize(24);
		text.setFillColor(sf::Color::White);
		text.setOutlineThickness(1);
		text.setOutlineColor(sf::Color::Black);
		text.setStyle(sf::Text::Bold | sf::Text::Regular);
		text.setString("");

	spin_threads(image);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				break;
			} else if (event.type == sf::Event::KeyPressed) {
				const sf::Keyboard::Key keycode = event.key.code;
				switch (keycode)
				{
					case sf::Keyboard::Escape:
						window.close();
						break;

					case sf::Keyboard::Space:
						text.setString("");
						break;

					case sf::Keyboard::F:
						if (&font1 == text.getFont()) {
							text.setFont(font2);
						} else {
							text.setFont(font1); 
						};
						break;

					default:
						break;
				};
			} else if (event.type == sf::Event::Resized) {
				window_width = event.size.width;
				window_height = event.size.height;
				ratio_GCD = std::__gcd(window_height, window_width);
				window.setView(sf::View(sf::FloatRect(0, 0, window_width, window_height)));
				join_threads();
				image = new sf::Uint8[window_height * window_width * 4];

				spin_threads(image);
				std::string message = "Current: " + std::to_string(window_width) + "x" + std::to_string(window_height) + ", GCD: " + std::to_string(ratio_GCD);
				text.setString(message);
			}
		};

		texture.update(image, window_width, window_height, 0, 0);


		window.clear();
		window.draw(sprite);		
		window.draw(text);
		window.display();
	};

	join_threads();

	return EXIT_SUCCESS;
}

int spin_threads(sf::Uint8 *image) {
	for (int t = 0; t < threads.size(); ++t) { 
		int lower_bound = t*window_height/amount_of_threads;
		int upper_bound = (t+1)==amount_of_threads ? window_height : (t+1)*window_height/amount_of_threads;
		int left_bound = 0;
		int right_bound = window_width;
		threads[t] = std::thread(render_chunk, image, lower_bound, upper_bound, left_bound, right_bound);
		fmt::print("Thread {} has started execution with id '{}'.\n", t, threads[t].get_id());
	};
	return EXIT_SUCCESS;
}

int join_threads() {
	for (int t = 0; t < amount_of_threads; ++t) {
		threads[t].join();
	};
	return EXIT_SUCCESS;
}

int render_chunk (sf::Uint8 *target_image, int lower_bound, int upper_bound, int left_bound, int right_bound) {
	for (int row = upper_bound-1; row >= lower_bound; row--) {
		for (int column = left_bound; column < right_bound; column++) {
			float r = float(column) / float(window_width);
			float g = float(row) / float(window_height);
			float b = (((column%ratio_GCD < 2)||(row%ratio_GCD < 2))&&(ratio_GCD > 15)) ? 0.3f : 0.6f;
			int shift = 4*((window_height-row-1)*window_width+column);
			target_image[shift+0] = int(255.99*r);
			target_image[shift+1] = int(255.99*g);
			target_image[shift+2] = int(255.99*b);
			target_image[shift+3] = int(255.99);
		} 
	}
	fmt::print("Thread {} has finished execution.\n", std::this_thread::get_id());
	return EXIT_SUCCESS;
}