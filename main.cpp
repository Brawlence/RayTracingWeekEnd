#define APP_NAME "SFML-RT"

#include "CLI11.hpp"			// command-line parsing
#include <SFML/Graphics.hpp> 	// SFML is 5 modules: system, window, graphics, network and audio; don't forget to include them in g++ -g ... -lsfml-window ... etc.
#include <fmt/format.h>			// somewhy won't compile under mingw with linker flag -lfmt; so use in header-only mode with -DFMT_HEADER_ONLY

#include "main.h"
#include "renderer.h"

static window_properties win_prop;

static sf::Vector2i mouse_pos;
static bool tracking = false;

int main(int argc, char *argv[]) {

	CLI::App app(APP_NAME);

	app.add_option("-h,--height",     win_prop.height,     "The height of the output picture");
	app.add_option("-w,--width",      win_prop.width,      "The width of the output picture");
	app.add_flag  ("-f,--fullscreen", win_prop.fullscreen, "Force fullscreen");

	CLI11_PARSE(app, argc, argv);

	sf::VideoMode screenSize = sf::VideoMode::getDesktopMode();
		win_prop.width = win_prop.fullscreen?screenSize.width:win_prop.width;
		win_prop.height = win_prop.fullscreen?screenSize.height:win_prop.height;
		screenSize = sf::VideoMode(win_prop.width, win_prop.height, screenSize.bitsPerPixel);

	sf::Uint32 window_style = (win_prop.fullscreen ? sf::Style::Fullscreen : sf::Style::Resize | sf::Style::Close);

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
	window.setFramerateLimit(win_prop.framerate_limiter);
	window.requestFocus();

	sf::Vector2u window_size = window.getSize();

	sf::Texture texture;
	if (!texture.create(win_prop.width, win_prop.height)) return EXIT_FAILURE;
	sf::Sprite sprite(texture);

	sf::Uint8* image = new sf::Uint8[win_prop.height * win_prop.width * 4];

	sf::Font font1, font2;
	if (!font1.loadFromFile("./Fonts/L.ttf")) return EXIT_FAILURE;		
	if (!font2.loadFromFile("./Fonts/CG.ttf")) return EXIT_FAILURE;

	sf::Text text;
	text.setFont(font1);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::White);
	text.setOutlineThickness(1);
	text.setOutlineColor(sf::Color::Black);
	text.setStyle(sf::Text::Bold | sf::Text::Regular);
	text.setString("");

	sf::CircleShape shape(20.f);
	shape.setFillColor(sf::Color::Blue);
	shape.setOutlineColor(sf::Color::White);
	shape.setOutlineThickness(1);
	shape.setPosition(50.f,50.f);

	spin_threads(image, win_prop);

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
						tracking = !tracking;
						tracking?text.setString("On"):text.setString("Off");
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
			} else if (event.type == sf::Event::MouseMoved) {
				if (tracking) {
					mouse_pos = sf::Mouse::getPosition(window);
					shape.setPosition(mouse_pos.x - shape.getRadius(), mouse_pos.y - shape.getRadius());
				};
			} else if (event.type == sf::Event::Resized) {
				win_prop.width = event.size.width;
				win_prop.height = event.size.height;
				win_prop.ratio_GCD = std::__gcd(win_prop.height, win_prop.width);
				window.setView(sf::View(sf::FloatRect(0, 0, win_prop.width, win_prop.height)));
				join_threads();
				image = new sf::Uint8[win_prop.height * win_prop.width * 4];

				spin_threads(image, win_prop);
				
				std::string message = fmt::format("Resolution: {}x{}, GCD: {}", win_prop.width, win_prop.height, win_prop.ratio_GCD);
				text.setString(message);
			}
		};

		texture.update(image, win_prop.width, win_prop.height, 0, 0);


		window.clear();
		window.draw(sprite);	
		window.draw(shape);	
		window.draw(text);
		window.display();
	};

	join_threads();

	return 0;
}