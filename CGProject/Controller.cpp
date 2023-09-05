#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>



//Class that represents the controller for the player character to handle its movement (horizontal and jumping) and health.
class Controller {
private :
	// Parameters
	const float FOVy = glm::radians(70.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 1000.f;

	// Player starting point, scaling and rotation
	const glm::vec3 StartingPosition = glm::vec3(-1,5.5f,10);
	const glm::quat StartingRotation =
		glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(0.0f), glm::vec3(0, 1, 0));
	float scalingFactor = .8f;

	// Camera target height and distance
	const float camHeight = 0.45;
	const float camDist = 5.f;

	// Camera Pitch limits
	const float minPitch = glm::radians(-60.0f);
	const float maxPitch = glm::radians(60.0f);
	// Rotation speed
	const float ROT_SPEED_ALPHA = glm::radians(160.0f);
	const float ROT_SPEED_BETA = glm::radians(120.0f);
	//Damping factor
	const float lambda = 10.0f;



	//World, Projection and view matrix
	glm::mat4 World, Prj, View;

	glm::vec3 Pos;
	glm::vec3 NewPos;
	glm::quat CharRot;
	glm::vec3 CamPos;
	glm::vec3 DampPos;
	glm::vec3 OldPos;
	float CamAlpha;
	float CamBeta;
	float CamRho;

	BoundingBox* bbCharacter;
	BoundingSphere* bsCharacter;
	//Vertical (rough) offset from the origin of the model (in the feet) and the ceneter of the geometry
	float charOriginOffset_y = 0.3f;

	//Movement
	const float MAX_ZONE = 0.7f;
	const float MAX_MOVE_SPEED = 6.5f;
	const float INITIAL_MOVE_SPEED = 1.7f;
	const float INITIAL_MOVE_SPEED_AIR = 1.3f;
	const float MOVE_FORCE = 7.f;
	const float MOVE_FORCE_AIR = 5.f;
	glm::vec3 currvelocity_h = glm::vec3(0, 0, -1);
	glm::vec3 lastvelocity_h = glm::vec3(0, 0, -1);
	glm::vec3 directionAtJump = glm::vec3(0.0f);
	glm::vec3 lastHorizontalDir = glm::vec3(0, 0, -1);

	//Jump
	bool wasGrounded = false;
	bool jumpWasPressed = false;
	bool peakReached = false;

	float jumpTime = 0.0f;
	float maxJumpTime = 0.5f;

	const float G = -9.81f * 2;
	const float JUMP_INSTANT_VELOCITY = 7.5f;
	const float JUMP_FORCE = 10.0f;
	float currvelocity_y = 0.0f;

	glm::vec3 stretch = glm::vec3(1);
	int stretchState = STRETCH_NONE;

	//Life
	const float MAX_HEALTH = 3;
	float health;
	const float INVINCIBILITY_TIME = 1.5;
	const float FLASHING_FREQ = 0.15f;
	float counterHitTime = 0;
	float lastHitTime = 0;

	//Game over
	const float GAME_OVER_DURATION = 2;
	float gameOverTime = 0;
	float shrink = 1;

	// Updates the stretch of the character based on the current state (landing and jumping).
	void updateStretch(int state, float deltaT) {
		//Time that makes up a half a period of the stretch function
		const static float timeStretchJumping = 0.15f;
		const static float timeStretchLanding = 0.2f;
		//How much the stretch extends
		const static float stretchJumpingRange = 0.4f;
		const static float stretchLandingRange = 0.5f;
		static float time = 0;
		static int lastState;
		// Resets time on change of state
		if (state != lastState) {
			lastState = state;
			time = 0;
		}
		else
			time += deltaT;

		if (state == STRETCH_JUMPING) {
			float offset = 3.14 / 2 + 1;
			if (time < timeStretchJumping)
				stretch = glm::vec3(1, glm::sin(time * ((2 * 3.14) / (timeStretchJumping * 2.f)) - offset) * stretchJumpingRange + 0.8, 1);
			else {
				stretch = glm::vec3(1, glm::sin(timeStretchJumping * ((2 * 3.14) / (timeStretchJumping * 2.f)) - offset) * stretchJumpingRange + 0.9, 1);
				stretchState = STRETCH_NONE;
			}

		}
		else if (state == STRETCH_LANDING) {
			float offset = 3.14;
			if (time < timeStretchLanding)
				stretch = glm::vec3(glm::sin(time * ((2 * 3.14) / (timeStretchLanding * 2.f))) * stretchLandingRange + 1,
									glm::sin(time * ((2 * 3.14) / (timeStretchLanding * 2.f)) - offset) * stretchLandingRange + 1,
									glm::sin(time * ((2 * 3.14) / (timeStretchLanding * 2.f))) * stretchLandingRange + 1);
			else {
				stretch = glm::vec3(glm::sin(timeStretchLanding * ((2 * 3.14) / (timeStretchLanding * 2.f))) * stretchLandingRange + 1,
									glm::sin(timeStretchLanding * ((2 * 3.14) / (timeStretchLanding * 2.f)) - offset) * stretchLandingRange + 1,
									glm::sin(timeStretchLanding * ((2 * 3.14) / (timeStretchLanding * 2.f))) * stretchLandingRange + 1);
				stretchState = STRETCH_NONE;
			}
		}
		else if (stretchState == STRETCH_NONE) {
			//Do nothing
		}
	}

public:

	Controller() {
		Pos = StartingPosition;
		NewPos = Pos;
		CharRot = StartingRotation;
		CamPos = StartingPosition + glm::vec3(0, camHeight, camDist);
		DampPos = CamPos;
		OldPos = CamPos;
		CamAlpha = 0.0f;
		CamBeta = 0.0f;
		CamRho = 0.0f;

		health = MAX_HEALTH;
	}

	void init(BoundingBox &bbChar, BoundingSphere &bsChar) {
		bbCharacter = &bbChar;
		bsCharacter = &bsChar;
	}

	void moveCharacterAndCamera(float Ar, float deltaT, glm::vec3 m, glm::vec3 r, bool jump, std::vector<BoundingBox *> collisionBoxes) {
		// Game Logic implementation

		//Joystick at most goes to 0.7 more or less, this causes max speed to never be reached unless using a keyboard. 
		//This fixes it by remapping from the -0.7/0.7 range to -1/1.
		m.x = glm::clamp(((m.x + MAX_ZONE) / (MAX_ZONE * 2)) * 2 - 1, -1.f, 1.f);
		m.z = glm::clamp(((m.z + MAX_ZONE) / (MAX_ZONE * 2)) * 2 - 1, -1.f, 1.f);
		//If the lenght is more than 1 we clamp it by normalizing, otherwise we keep it as is to allow incremental and precise control of the character's speed.
		m = glm::length(m) < 1 ? m : glm::normalize(m);

		//---Move character---

		//Horizontal Movement
		glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
		glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);
		glm::vec3 horizontalDir = glm::vec3(0);

		if (m.x != 0 || m.z != 0) {
			//Current inputted horizontal direction in which we are to move, based on current character orientation and axis inputted. 
			// Normalized so that speed is consistent in all directions. 
			// Scaled by the lenght of the inputted axis to have progressive speed based on joystick position.
			horizontalDir = glm::normalize(ux * m.x + uz * m.z) * glm::length(m);

			//On ground
			if (wasGrounded) {
				//If the velocity is less than the minimum (relative to the amount that the joystick was moved in, which was scaled by the lenght of m)
				//Then we start by moving by this minimum speed, otherwise we increase the velocity using a force
				currvelocity_h = glm::length(currvelocity_h) < glm::length(INITIAL_MOVE_SPEED * horizontalDir) ?
						INITIAL_MOVE_SPEED * horizontalDir : 
						currvelocity_h + MOVE_FORCE * horizontalDir * glm::length(currvelocity_h) * deltaT;
			}

			//In the air
			else {
				//If we jumped in place we take the direction in which the character is currently trying to move as the jumping direction
				if (directionAtJump == glm::vec3(0))
					directionAtJump = horizontalDir;

				//We calculate the dot product between the inputted direction and the direction where we jumped to have different behavior depending on how much we try to steer
				float changeDirAir = glm::dot(horizontalDir, directionAtJump);
				//If we are roughly keeping the same direction we simply update it similarly as we did to the grounded case
				if (changeDirAir >= 0.7) {
					currvelocity_h = glm::length(currvelocity_h) < glm::length(INITIAL_MOVE_SPEED_AIR * horizontalDir) ?
						INITIAL_MOVE_SPEED_AIR * horizontalDir :
						currvelocity_h + MOVE_FORCE_AIR * horizontalDir * glm::length(currvelocity_h) * deltaT;
				}
				//If we are trying to steer left or right, we update it similarly, but we linearly interpolate the inputted direction with the original jumping direction.
				// Basically, we want the jump to feel committed to the initial direction but also to allow leniency in steering slightly and to make this steer feel "weightier".
				// To find the actual moving direction, we interpolate using the dot between the current inputted direction and the previous horizontal direction.
				// When they are very different (opposite in direction), the dot will be low and the actual moving direction closer to the original jumping direction.
				// The longer we steer for, the more the current inputted direction and the previous moving direction will be similar, leading to an higher dot product, meaning to an
				// actual moving direction closer to the inputted direction.
				// A large steer will naturally take longer before the actual moving direction is equal to the inputted direction.
				else if (changeDirAir <0.7 && changeDirAir > -0.7) {
					//We bias our choice to the jump direction for tighter controls
					float bias = -0.2f;
					//We must keep the magnitude of the direction intact after interpolating to have movement that reflects the inputs correctly
					float lenght = glm::length(horizontalDir);
					horizontalDir = glm::mix(directionAtJump, horizontalDir, glm::clamp((glm::dot(horizontalDir, lastHorizontalDir) + 1)/2 + bias, 0.f, 1.f));
					horizontalDir = glm::normalize(horizontalDir) * lenght;
					currvelocity_h = glm::length(currvelocity_h) < glm::length(INITIAL_MOVE_SPEED_AIR * horizontalDir) ?
						INITIAL_MOVE_SPEED_AIR * horizontalDir : 
						currvelocity_h + MOVE_FORCE_AIR * horizontalDir * glm::length(currvelocity_h) * deltaT;
				}
				//If we are completely changing direction, we instead increase to force to have a faster stopping and changing of direction, which leads to better control
				else {
					currvelocity_h = glm::length(currvelocity_h) < glm::length(INITIAL_MOVE_SPEED_AIR * horizontalDir) ?
						INITIAL_MOVE_SPEED_AIR * horizontalDir :
						currvelocity_h + (MOVE_FORCE_AIR * 4 / 3) * horizontalDir * glm::length(currvelocity_h) * deltaT;
				}
			}

			//Finally we clamp the velocity to the maximim allowed (always based on how much the joystick was tilted)
			currvelocity_h = glm::length(currvelocity_h) > glm::length(MAX_MOVE_SPEED * horizontalDir) ? 
				glm::normalize(currvelocity_h) * glm::length(MAX_MOVE_SPEED * horizontalDir) :
				currvelocity_h;
			
			//Update last horizontal direction
			if(horizontalDir != glm::vec3(0))
				lastHorizontalDir = horizontalDir;
		}
		else {
			//If no direction was inputted, stop immediately when grounded for precise jumps
			if(wasGrounded) 
				currvelocity_h = glm::vec3(0);
			//If jumping we keep the momentum instead
			else 
				currvelocity_h = currvelocity_h;
		}
		//Update position
		NewPos += currvelocity_h * deltaT;
		
		//Check if we collided by moving horizontally with rayCasting
		bool hit = false;
		glm::vec3 currMovingDir = glm::normalize(currvelocity_h);
		glm::vec3 currOrthoMovingDir = glm::normalize(glm::cross(currMovingDir, glm::vec3(0, 1, 0)));
		for (int i = 0; i < collisionBoxes.size() && !hit; i++)
		{
			if (collisionBoxes.at(i)->canCollide) {
				//Considers a PLANE that rotates with character current moving direction
				hit = rayCollision(NewPos + bbCharacter->pos + currMovingDir * bbCharacter->halfExtents.z,
					lastHorizontalDir, 0.01f, collisionBoxes.at(i));
				hit = hit || rayCollision(NewPos + bbCharacter->pos + currMovingDir * bbCharacter->halfExtents.z + currOrthoMovingDir * bbCharacter->halfExtents.x,
					lastHorizontalDir, 0.01f, collisionBoxes.at(i));
				hit = hit || rayCollision(NewPos + bbCharacter->pos + currMovingDir * bbCharacter->halfExtents.z + currOrthoMovingDir * (bbCharacter->halfExtents.x / 2.f),
					lastHorizontalDir, 0.01f, collisionBoxes.at(i));
				hit = hit || rayCollision(NewPos + bbCharacter->pos + currMovingDir * bbCharacter->halfExtents.z + currOrthoMovingDir * (-bbCharacter->halfExtents.x),
						lastHorizontalDir, 0.01f, collisionBoxes.at(i));
				hit = hit || rayCollision(NewPos + bbCharacter->pos + currMovingDir * bbCharacter->halfExtents.z + currOrthoMovingDir * (-bbCharacter->halfExtents.x / 2.f),
					lastHorizontalDir, 0.01f, collisionBoxes.at(i));
			}
			else
				hit = false;
		}
		if (!hit) { //Update position

			Pos.x = NewPos.x;
			Pos.z = NewPos.z;
		}
		else { //Rollback
			NewPos.x = Pos.x;
			NewPos.z = Pos.z;
			if(wasGrounded)
				currvelocity_h = glm::vec3(0.0f);
		}

		//Vertical Movement
		glm::vec3 uy = glm::vec3(0, 1, 0);

		bool grounded = false;
		int ground = -1;
		//We chack if we are grounded first by rayCasting
		if (currvelocity_h != glm::vec3(0)) {
			lastvelocity_h = currvelocity_h;
		}
		glm::vec3 lastMovingDir = glm::normalize(lastvelocity_h);
		glm::vec3 lastOrthoMovingDir = glm::normalize(glm::cross(lastMovingDir, glm::vec3(0, 1, 0)));
		for (int i = 0; i < collisionBoxes.size() && !grounded; i++)
		{
			if (collisionBoxes.at(i)->canCollide) {
				int numPoints = 0;
				bool grounded1 = rayCollision(NewPos + lastMovingDir * bbCharacter->halfExtents.z + lastOrthoMovingDir * bbCharacter->halfExtents.x,
					glm::vec3(0, -1, 0), 0.01f, collisionBoxes.at(i));
				bool grounded2 = rayCollision(NewPos + lastMovingDir * bbCharacter->halfExtents.z + lastOrthoMovingDir * -bbCharacter->halfExtents.x,
					glm::vec3(0, -1, 0), 0.01f, collisionBoxes.at(i));
				bool grounded3 = rayCollision(NewPos + lastMovingDir * -bbCharacter->halfExtents.z + lastOrthoMovingDir * bbCharacter->halfExtents.x,
					glm::vec3(0, -1, 0), 0.01f, collisionBoxes.at(i));
				bool grounded4 = rayCollision(NewPos + lastMovingDir * -bbCharacter->halfExtents.z + lastOrthoMovingDir * -bbCharacter->halfExtents.x,
					glm::vec3(0, -1, 0), 0.01f, collisionBoxes.at(i));
				numPoints = grounded1 ? numPoints + 1 : numPoints;
				numPoints = grounded2 ? numPoints + 1 : numPoints;
				numPoints = grounded3 ? numPoints + 1 : numPoints;
				numPoints = grounded4 ? numPoints + 1 : numPoints;
				//We are grounded if at least two edges of the character base are on the ground (which avoids clipping because of the charachter rotation)
				grounded = numPoints > 1;
				ground = i;

			}
			else
				grounded = false;
		}

		//Jump FSA based on current and previous jumping state
		// 
		//Start jump with an initial velocity and setting parameters
		if (grounded && jump && !jumpWasPressed) { 
			currvelocity_y = JUMP_INSTANT_VELOCITY;
			NewPos += uy * deltaT * (currvelocity_y + ((G + JUMP_FORCE) * deltaT));

			jumpTime = 0;
			peakReached = false;
			stretchState = STRETCH_JUMPING;
			directionAtJump = horizontalDir;
		}
		//Keep jumping if button is continuously pressed by appling a force going up
		else if (jump && jumpWasPressed && jumpTime + deltaT < maxJumpTime) { 
			jumpTime += deltaT;
			currvelocity_y += (G + JUMP_FORCE) * deltaT;
			NewPos += uy * deltaT * (currvelocity_y + ((G + JUMP_FORCE) * deltaT));
		}
		//Stop jump when peak is reached after a fixed time
		else if(!peakReached && jump && jumpWasPressed) 
		{
			peakReached = true;
			float timeLeft = maxJumpTime - jumpTime;
			float timeExtra = (jumpTime + deltaT) - maxJumpTime;

			//End rising jump
			currvelocity_y += (G + JUMP_FORCE) * timeLeft;
			NewPos += uy * timeLeft * (currvelocity_y + ((G + JUMP_FORCE) * timeLeft));

			//And start falling applying gravity
			currvelocity_y += G * timeExtra;
			NewPos += uy * timeExtra * (currvelocity_y + (G * timeExtra));
		}
		//Fall by applying gravity if not on ground (and not pressing jump or the peak was reached)
		else if(!grounded)
		{
			currvelocity_y += G * deltaT;
			NewPos += uy * deltaT* (currvelocity_y + (G * deltaT));
		}
		//Landing frame. We reposition the character at ground level manually so that its position is consistent
		// Otherwise we would need some sort of continuous collision detection or to retrieve the collision point
		else if (grounded && !wasGrounded) 
		{
			stretchState = STRETCH_LANDING;
			//On ground
			currvelocity_y = 0;
			//Place object back terrain level in order to avoid clipping though and to have consistent terrain levels
			NewPos.y = ((*collisionBoxes[ground]->mMat) * glm::vec4(collisionBoxes[ground]->pos + collisionBoxes[ground]->halfExtents, 1)).y + 0.001f;
		}
		//Stay on ground (actually the same as above right now but it could be useful to have them distinct
		else if (grounded) 
		{
			stretchState = STRETCH_LANDING;
			//On ground
			currvelocity_y = 0;
			//Place object back terrain level in order to avoid clipping though and to have consistent terrain levels
			NewPos.y = ((*collisionBoxes[ground]->mMat) * glm::vec4(collisionBoxes[ground]->pos + collisionBoxes[ground]->halfExtents,1)).y + 0.001f;
		}
		wasGrounded = grounded;
		Pos.y = NewPos.y;
		jumpWasPressed = jump;
		
		//Update the stretch of the model to convey the jumping and landing feeling
		updateStretch(stretchState, deltaT);

		//For debug
		//std::cout << "Pos: " << Pos.x << " " << Pos.y << " " << Pos.z << "\n";
		// 
		
		//---Rotate character---
		float targetCharYaw;
		bool shouldRotate = true;
		if (m.x != 0 || m.z != 0) {
			targetCharYaw = -atan2(m.x, m.z); //The direction the player wishes to move its character in
			targetCharYaw += CamAlpha; //Since we have moved the charachter relative to camera orientation, adding CamAlpha adjusts the direction the charachter must be rotated to face it's actual direction
		}
		else
			shouldRotate = false;

		glm::quat interpolatedQuat = glm::slerp(CharRot, glm::rotate(glm::quat(1, 0, 0, 0), targetCharYaw, glm::vec3(0, 1, 0)), 0.2f);
		if (shouldRotate)
			CharRot = interpolatedQuat;

		//---Flashing if hurt---
		counterHitTime += deltaT;
		float flash = counterHitTime - lastHitTime < INVINCIBILITY_TIME -0.3f ?
			(int)glm::floor((counterHitTime - lastHitTime) / FLASHING_FREQ) % 2 :
			1;

		//------WORLD MATRIX------
		World = glm::translate(glm::mat4(1), Pos) *
				glm::mat4(CharRot) *
				glm::scale(glm::mat4(1), glm::vec3(scalingFactor * stretch * flash));

		//------Move camera-------
		CamAlpha = CamAlpha - ROT_SPEED_ALPHA * deltaT * r.y;
		CamBeta = CamBeta - ROT_SPEED_BETA * deltaT * r.x;
		CamBeta = CamBeta < minPitch ? minPitch :
			(CamBeta > maxPitch ? maxPitch : CamBeta);

		CamPos = glm::vec3(0, camHeight, camDist); //Starting camera position, now we find actual position by rotating first and then translating
		glm::mat4 CamTransform =
			glm::translate(glm::mat4(1.0f), Pos) *
			glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), CamBeta, glm::vec3(-1, 0, 0));
		CamPos = CamTransform * glm::vec4(CamPos.x, CamPos.y, CamPos.z, 1);

		DampPos = OldPos * exp(-lambda * deltaT) + CamPos * (1 - exp(-lambda * deltaT)); //We apply damping
		OldPos = DampPos;

		//------VIEW MATRIX------
		View = glm::rotate(glm::mat4(1.0f), CamRho, glm::vec3(0, 0, 1)) *
			glm::lookAt(DampPos, Pos + glm::vec3(0, camHeight, 0), glm::vec3(0, 1, 0));

		//------PROJECTION MATRIX------
		Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;
	}

	// When hit, character looses a life and must start flashing (by setting the hit time)
	// Returns true if character is actually hit (meaning, he is not invincible)
	bool onHit(float hitTime) {
		if (hitTime - lastHitTime > INVINCIBILITY_TIME) {
			health--;
			lastHitTime = hitTime;
			counterHitTime = hitTime;
			return true;
		}
		else
			return false;
	}

	// Sets gameover time
	void initGameOver(float time){
		gameOverTime = time;
	}

	// Performs death animation
	bool gameOver(float time) {
		shrink = glm::mix(1.0f, 0.0f, (time - gameOverTime) / GAME_OVER_DURATION);
		World = glm::translate(glm::mat4(1), Pos) *
				glm::mat4(CharRot) *
				glm::scale(glm::mat4(1), glm::vec3(scalingFactor * shrink));
		return (time - gameOverTime) > GAME_OVER_DURATION;
	}

	// Resets all parameters to restart play
	void reset() {
		Pos = StartingPosition;
		NewPos = Pos;
		CharRot = StartingRotation;
		CamPos = StartingPosition + glm::vec3(0, camHeight, camDist);
		DampPos = CamPos;
		OldPos = CamPos;
		CamAlpha = 0.0f;
		CamBeta = 0.0f;
		CamRho = 0.0f;

		health = MAX_HEALTH;
		currvelocity_h = glm::vec3(0);
		currvelocity_y = 0;

		lastHitTime = 0;
	}

	// GETTERS
	glm::mat4 getWorld() {
		return World;
	}

	glm::mat4 getPrj() {
		return Prj;
	}

	glm::mat4 getView() {
		return View;
	}

	glm::vec3 getCamPos() {
		return DampPos;
	}

	float getCharOriginOffset_y() {
		return charOriginOffset_y;
	}

	float getHealth() {
		return health;
	}

	glm::quat getcharRot() {
		return CharRot;
	}
};


//Class that represents an enemy that moves around and can damage the player
class Enemy {
private:
	// Player starting point, scaling and rotation
	glm::vec3 startingPosition;
	float startingRotation;
	float startingScaling;
	glm::vec3 moveDirection;
	float moveDistance;
	float moveSpeed;

	glm::vec3 endPosition;

	glm::vec3 currPosition;
	float currRot;
	float currScale;

	BoundingBox* bbEnemy;

	float timeElapsed = 0;

	glm::mat4 World;

public:

	void init(BoundingBox& bb, glm::vec3 startPos, float startRot, glm::vec3(startScale), glm::vec3 dir, float distance, float speed) {
		bbEnemy = &bb;
		startingPosition = startPos;
		startingRotation = startRot;
		startingScaling = startScale.x;
		moveDirection = glm::normalize(dir);
		moveDistance = distance;
		moveSpeed = speed;

		endPosition = startingPosition + moveDirection * moveDistance;
		currScale = startingScaling;
	}

	// Moves enemy on a line using the parameters set
	void moveEnemy(float deltaT) {
		currPosition = glm::mix(startingPosition, endPosition, (float)(glm::sin(timeElapsed * ((2 * 3.14) * moveSpeed)) + 1)/2.0f);
		currRot = glm::mix(startingRotation, startingRotation + 360.0f, (float)timeElapsed * moveSpeed);
		timeElapsed += deltaT;

		World = glm::translate(glm::mat4(1), currPosition)*
			glm::rotate(glm::mat4(1), glm::radians(currRot), glm::vec3(0, 1, 0))*
			glm::scale(glm::mat4(1), glm::vec3(currScale));
	}

	// Check collision with the character bounding sphere
	bool checkCollision(BoundingSphere bsChar) {
		return sphereBoxCollision(*bbEnemy, bsChar);
	}

	// GETTERS
	glm::mat4 getWorld() {
		return World;
	}

};

//Class that represents a scroll that can be taken by the character as an objective
class Scroll {
private:
	// Player starting point, scaling and rotation
	glm::vec3 startingPosition;
	glm::vec3 startingRotation;
	float startingScaling;

	float bounceDist = 1;
	glm::vec3 endPosition;

	glm::vec3 currPosition;
	glm::vec3 currRot;
	float currScale;

	float rotSpeed = 0.2;
	float bounceSpeed = 0.3f;

	BoundingBox* bbScroll;

	float timeCaptured = 0;
	float timeElapsed = 0;
	float TIME_ANIMATION = 2.f;

	bool captured = false;

	glm::mat4 World;

public:

	void init(BoundingBox& bb, glm::vec3 startPos, glm::vec3 startRot, glm::vec3 startScale) {
		bbScroll= &bb;
		startingPosition = startPos;
		startingRotation = startRot;
		startingScaling = startScale.x;

		endPosition = startingPosition + glm::vec3(0, 1, 0) * bounceDist;
		currScale = startingScaling;
		currRot = startRot;
	}

	// Moves scroll up and down and rotates it for a nice visual effect. If taken, it will slowly fade away
	void updateScroll(float deltaT, float time) {
		if (currScale != 0) {
			currPosition = glm::mix(startingPosition, endPosition, (float)(glm::sin(timeElapsed * ((2 * 3.14) * bounceSpeed)) + 1) / 2.0f);
			currRot.y = glm::mix(startingRotation.y, startingRotation.y + 360.0f, (float)timeElapsed * rotSpeed);
			if (captured)
				currScale = glm::clamp(glm::mix(startingScaling, 0.0f, (time - timeCaptured) / TIME_ANIMATION), 0.f, 1.f);
			timeElapsed += deltaT;

			World = glm::translate(glm::mat4(1), currPosition)*
					glm::rotate(glm::mat4(1), glm::radians(currRot.y), glm::vec3(0, 1, 0))*
					glm::rotate(glm::mat4(1), glm::radians(currRot.x), glm::vec3(1, 0, 0))*
					glm::rotate(glm::mat4(1), glm::radians(currRot.z), glm::vec3(0, 0, 1))*
					glm::scale(glm::mat4(1), glm::vec3(currScale));
		}
	}

	// Check collision with the character bounding sphere
	bool checkCollision(BoundingSphere bsChar) {
		return sphereBoxCollision(*bbScroll, bsChar);
	}

	// Called when the scroll was taken by the player. When caputred it will slowly fade
	void getScroll(float time) {
		if(! captured)
			timeCaptured = time;
		captured = true;
		rotSpeed = 1.2;
	}

	void reset() {
		currPosition =startingPosition;
		currRot = startingRotation;
		currScale = startingScaling;

		captured = false;
		rotSpeed = 0.2;
	}

	// GETTERS
	glm::mat4 getWorld() {
		return World;
	}

};