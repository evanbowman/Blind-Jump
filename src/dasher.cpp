//========================================================================//
// Copyright (C) 2016 Evan Bowman                                         //
//                                                                        //
// This program is free software: you can redistribute it and/or modify   //
// it under the terms of the GNU General Public License as published by   //
// the Free Software Foundation, either version 3 of the License, or      //
// (at your option) any later version.                                    //
//                                                                        //
// This program is distributed in the hope that it will be useful,        //
// but WITHOUT ANY WARRANTY; without even the implied warranty of         //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
// GNU General Public License for more details.                           //
//                                                                        //
// You should have received a copy of the GNU General Public License      //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.  //
//========================================================================//

#include "dasher.hpp"
#include <cmath>
#include "angleFunction.hpp"

Dasher::Blur::Blur(sf::Sprite * spr, float xInit, float yInit) {
	this->spr = *spr;
	this->xInit = xInit;
	this->yInit = yInit;
	killflag = false;
	timer = 0;
	sf::Color c = this->spr.getColor();
	c.a -= 120;
	c.r -= 30;
	c.g -= 30;
	c.b -= 10;
	this->spr.setColor(c);
}

sf::Sprite * Dasher::Blur::getSprite() {
	return &spr;
}

void Dasher::Blur::update(const sf::Time & elapsedTime, float xOffset, float yOffset) {
	timer += elapsedTime.asMilliseconds();
	spr.setPosition(xInit + xOffset, yInit + yOffset);
	if (timer > 18) {
		timer = 0;
		sf::Color c = spr.getColor();
		if (c.a > 30) {
			c.a -= 30;
			spr.setColor(c);
		} else
			killflag = true;
	}
}

bool Dasher::Blur::getKillFlag() {
	return killflag;
}

const Dasher::HBox & Dasher::getHitBox() const {
	return hitBox;
}

Dasher::Dasher(const sf::Texture & mainTxtr, float _xInit, float _yInit, float _ppx, float _ppy)
	: Enemy{_xInit, _yInit, _ppx, _ppy},
	  shotCount{0},
	  state{State::idle},
	  dasherSheet{mainTxtr},
	  deathSheet{mainTxtr},
	  hSpeed{0.f},
	  vSpeed{0.f},
	  timer{0}
{
	dasherSheet.setOrigin(14, 8);
	deathSheet.setOrigin(14, 8);
	shadow.setTexture(mainTxtr);
	shadow.setTextureRect(sf::IntRect(0, 100, 18, 16));
	health = 5;
}

const sf::Sprite & Dasher::getSprite() const {
	switch(state) {
	case State::dying:
		return deathSheet[frameIndex];
			
	case State::dead:
		return deathSheet[14];

	default:
		return dasherSheet[frameIndex];
	}
}

const sf::Sprite & Dasher::getShadow() const {
	return shadow;
}

void Dasher::facePlayer() {
	if (xPos > playerPosX)
		dasherSheet.setScale(1, 1);
	else
		dasherSheet.setScale(-1, 1);
}

void Dasher::update(float xOffset, float yOffset, const std::vector<wall> & walls, EffectGroup & effects, const sf::Time & elapsedTime) {
	Enemy::update(xOffset, yOffset, walls, effects, elapsedTime);
	if (health > 0) {
		for (auto & element : effects.get<9>()) {
			if (hitBox.overlapping(element.getHitBox()) && element.checkCanPoof()) {
				if (health == 1) {
					element.disablePuff();
					element.setKillFlag();
				}
				element.poof();
				health -= 1;
				colored = true;
				colorAmount = 1.f;
			}
		}
		if (health == 0) {
			onDeath(effects);
		}
	}
	Enemy::updateColor(elapsedTime);
	
	dasherSheet.setPosition(xPos + 4, yPos);
	deathSheet.setPosition(xPos + 4, yPos);
	shadow.setPosition(xPos - 4, yPos + 22);
	hitBox.setPosition(xPos, yPos);
	
	timer += elapsedTime.asMilliseconds();
	
	switch(state) {
	case State::idle:
		if (timer >= 200) {
			timer -= 200;
			const int select = std::abs(static_cast<int>(globalRNG())) % 2;
			if (select) {
				state = State::dashBegin;
				frameIndex = 1;
			} else {
				state = State::shootBegin;
				frameIndex = 3;
			}
		}
		break;

	case State::pause:
		if (timer >= 200) {
			timer -= 200;
			state = State::dashBegin;
			frameIndex = 1;
		}
		break;

	case State::shooting:
		facePlayer();
		frameTimer += elapsedTime.asMilliseconds();
		if (frameTimer > 80 && shotCount < 3) {
			frameTimer -= 80;
			shotCount++;
			if (xPos > playerPosX) {
				effects.add<0>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects), xInit - 14, yInit + 2);
				effects.add<7>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects),
							   globalResourceHandler.getTexture(ResourceHandler::Texture::redglow),
							   xInit - 12, yInit, angleFunction(playerPosX, playerPosY, xPos, yPos));
			} else {
				effects.add<0>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects), xInit + 6, yInit + 2);
				effects.add<7>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects),
							   globalResourceHandler.getTexture(ResourceHandler::Texture::redglow),
							   xInit + 4, yInit, angleFunction(playerPosX, playerPosY, xPos, yPos));
			}
		}

		if (timer > 300) {
			timer -= 300;
			shotCount = 0;
			state = State::pause;
		}
		break;

	case State::shootBegin:
		facePlayer();
		if (timer > 80) {
			timer -= 80;
			frameTimer = 0;
			state = State::shooting;
			frameIndex = 4;
		}
		break;

	case State::dashBegin:
    begin:
		facePlayer();
		if (timer > 352) {
			timer -= 352;
			state = State::dashing;
			frameIndex = 2;
			uint8_t tries{0};
			float dir{static_cast<float>(std::abs(static_cast<int>(globalRNG())) % 359)};
			do {
				tries++;
				if (tries > 254) {
					state = State::shootBegin;
					frameIndex = 3;
				    goto begin;
				}
				dir += 12;
			} while (wallInPath(walls, dir, xPos, yPos));
			hSpeed = 5 * cos(dir);
			vSpeed = 5 * sin(dir);
			if (hSpeed > 0) {
				dasherSheet.setScale(-1, 1);
				deathSheet.setScale(-1, 1);
			} else {
				dasherSheet.setScale(1, 1);
				dasherSheet.setScale(1, 1);
			}
		}
		break;

	case State::dashing:
		frameTimer += elapsedTime.asMilliseconds();
		if (frameTimer > 40) {
			frameTimer = 0;
			blurEffects.emplace_back(dasherSheet.getSpritePtr(), xInit, yInit);
		}
		if (timer > 250) {
			timer -= 250;
			state = State::dashEnd;
			frameIndex = 1;
			hSpeed = 0.f;
			vSpeed = 0.f;
		}
		
		if (Enemy::checkWallCollision(walls, 48, xPos, yPos)) {
			hSpeed *= -1.f;
			vSpeed *= -1.f;
		}
		break;

	case State::dashEnd:
		if (timer > 150) {
			blurEffects.clear();
			timer -= 150;
			state = State::idle;
			frameIndex = 0;
		}
		break;

	case State::dying:
		frameTimer += elapsedTime.asMilliseconds();
		if (frameTimer > 53) {
			frameTimer -= 53;
			frameIndex++;
			if (frameIndex > 14)
				state = State::dead;
		}
		break;

	case State::dead:
		break;
	}

	if (!blurEffects.empty()) {
		for (auto it = blurEffects.begin(); it != blurEffects.end();) {
			if (it->getKillFlag())
				it = blurEffects.erase(it);
			else {
				it->update(elapsedTime, xOffset, yOffset);
				++it;
			}
		}
	}
	
	xInit += hSpeed * (elapsedTime.asMilliseconds() / 17.6f);
	yInit += vSpeed * (elapsedTime.asMilliseconds() / 17.6f);
}

void Dasher::onDeath(EffectGroup & effects) {
	state = State::dying;
	hSpeed = 0;
	vSpeed = 0;
	frameIndex = 0;
	unsigned long int temp = std::abs(static_cast<int>(globalRNG())) % 4;
	if (temp == 0) {
	    effects.add<4>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects),
					   globalResourceHandler.getTexture(ResourceHandler::Texture::redglow),
					   xInit, yInit + 4, Powerup::Type::Heart);
	} else {
	    effects.add<5>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects),
					   globalResourceHandler.getTexture(ResourceHandler::Texture::blueglow),
					   xInit, yInit + 4, Powerup::Type::Coin);
	}
    effects.add<1>(globalResourceHandler.getTexture(ResourceHandler::Texture::gameObjects),
				   globalResourceHandler.getTexture(ResourceHandler::Texture::fireExplosionGlow),
				   xInit, yInit - 2);
	blurEffects.clear();
}

Dasher::State Dasher::getState() const {
	return state;
}

std::vector<Dasher::Blur> * Dasher::getBlurEffects() {
	return &blurEffects;
}

const sf::Vector2f & Dasher::getScale() const {
	return deathSheet.getScale();
}
