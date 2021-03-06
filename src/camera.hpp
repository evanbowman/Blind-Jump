#pragma once
#include "math.hpp"
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <utility>
#include <vector>

class Player;

class Camera {
    Player * pTarget;
    sf::View overworldView, windowView;
    sf::Vector2f startPosition, midpoint, buffer, currentPosition;
    sf::Vector2u windowSize;
    bool isShaking;
    uint8_t shakeIndex;
    int64_t shakeTimer, trackingTimer;
    float shakeIntensity;
    enum class State { trackMidpoint, followPlayer, foundEnemy };
    State state;
    void upscaleWindowView();

public:
    Camera(Player * _pTarget, const sf::Vector2f & viewPort,
           const sf::Vector2u &);
    void update(const sf::Time &, const std::vector<sf::Vector2f> &);
    void snapToTarget();
    void panDown();
    const sf::View & getOverworldView() const;
    const sf::View & getWindowView() const;
    void shake(float);
    void setOverworldView(const sf::View &);
    void setWindowView(const sf::View &);
    bool moving() const;
    sf::Vector2f getOffsetFromStart() const;
    sf::Vector2f getOffsetFromTarget() const;
};
