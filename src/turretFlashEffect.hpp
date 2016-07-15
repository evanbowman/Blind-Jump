//
//  turretFlashEffect.hpp
//  Blind Jump
//
//  Created by Evan Bowman on 10/20/15.
//  Copyright © 2015 Evan Bowman. All rights reserved.
//

#pragma once
#ifndef turretFlashEffect_hpp
#define turretFlashEffect_hpp

#include "SFML/Graphics.hpp"
#include "spriteSheet.hpp"
#include "Effect.hpp"

class TurretFlashEffect : public Effect {
private:
	mutable SpriteSheet<0, 116, 16, 16> spriteSheet;
	
public:
	TurretFlashEffect(const sf::Texture &, float, float);
	void update(float, float, const sf::Time &);
	const Sprite & getSprite() const;
};

#endif /* turretFlashEffect_hpp */
