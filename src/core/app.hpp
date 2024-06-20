#ifndef EMP_APP_HPP
#define EMP_APP_HPP
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "math/math_defs.hpp"
#include <iostream>
#include <unordered_map>
namespace emp {
class App {
    sf::Clock deltaClock;
public:
    struct KeyState {
        bool held = false;
        bool pressed = false;
        bool released = false;
    };
    std::unordered_map<sf::Keyboard::Key, KeyState> keys;
    std::unordered_map<sf::Mouse::Button, KeyState> buttons;
    vec2f mouse_position;

    vec2f window_size = {800, 600};
    virtual bool setup(sf::Window& window) {
        return true;
    }
    virtual void update(sf::Time deltaTime) {}
    virtual void render(sf::RenderWindow& rw) {}
    virtual void cleanup() {}

    void resetKeys() {
        for(auto& state : keys) {
            state.second.pressed = false;
            state.second.released = false;
        }
        for(auto& state : buttons) {
            state.second.pressed = false;
            state.second.released = false;
        }
    }
    void handleEvent(const sf::Event& event) {
        if(event.type == sf::Event::KeyPressed) {
            if(!keys[event.key.code].held) {
                keys[event.key.code].pressed = true;
            }else {
                keys[event.key.code].pressed = false;
            }
            keys[event.key.code].held = true;
        }else if (event.type == sf::Event::KeyReleased) {
            keys[event.key.code].held = false;
            keys[event.key.code].pressed = false;
            keys[event.key.code].released = true;
        }else {
            keys[event.key.code].released = false;
            keys[event.key.code].pressed = false;
        }

        if(event.type == sf::Event::MouseButtonPressed) {
            if(!buttons[event.mouseButton.button].held) {
                buttons[event.mouseButton.button].pressed = true;
            }else {
                buttons[event.mouseButton.button].pressed = false;
            }
            keys[event.key.code].held = true;
        }else if (event.type == sf::Event::MouseButtonReleased) {
            buttons[event.mouseButton.button].held = false;
            buttons[event.mouseButton.button].pressed = false;
            buttons[event.mouseButton.button].released = true;
        }else {
            buttons[event.mouseButton.button].released = false;
        }
    }

    
    inline friend void run(App& app) {
        sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(app.window_size.x, app.window_size.y), "My window");
        window.setFramerateLimit(60);
        if(!app.setup(window)) {
            return;
        }
        sf::Clock deltaClock;
        while (window.isOpen())
        {
            app.mouse_position = (vec2f)sf::Mouse::getPosition(window);
            sf::Time dt = deltaClock.restart();
            sf::Event event;
            vec2f mouse_pos = (vec2f)sf::Mouse::getPosition(window);

            app.resetKeys();
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
                app.handleEvent(event);
            }
            app.update(dt);
            window.clear(sf::Color::Black);
            app.render(window);
            window.display();
        }
    }
};
};
#endif
