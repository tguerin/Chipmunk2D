/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "chipmunk/chipmunk.h"
#include "chipmunk/chipmunk_unsafe.h"
#include "ChipmunkDemo.h"

// Store particle data
typedef struct {
	cpBody *body;
	cpShape *shape;
	cpFloat radius;
} Particle;

static Particle particles[10];
static int particle_count = 0;
static int step_count = 0;
static cpFloat floor_y = 0.0f;
static cpFloat ceiling_y = 0.0f;
static cpFloat half_w = 0.0f;
static cpFloat half_h = 0.0f;

static void
update(cpSpace *space, double dt)
{
	step_count++;
	
	// Sub-step to prevent tunneling through walls
	const cpFloat maxSubStep = 1.0f / 120.0f;
	int numSubSteps = (int)ceil(dt / maxSubStep);
	cpFloat subStep = dt / (cpFloat)numSubSteps;
	
	for(int i = 0; i < numSubSteps; i++){
		cpSpaceStep(space, subStep);
	}
}

static cpSpace *
init(void)
{
	half_w = 320.0f;
	half_h = 240.0f;
	
	cpSpace *space = cpSpaceNew();
	cpSpaceSetGravity(space, cpv(0, -980));
	
	cpSpaceSetIterations(space, 10);
	cpSpaceSetCollisionSlop(space, 0.1f);
	cpSpaceSetCollisionBias(space, pow(1.0 - 0.1, 60.0));
	cpSpaceSetCollisionPersistence(space, 3);
	cpSpaceSetDamping(space, 0.99f); // Prevent infinite bouncing
	cpSpaceSetSleepTimeThreshold(space, 0.5f); // Enable sleeping for better performance
	cpSpaceSetIdleSpeedThreshold(space, 0.0f);
	
	cpBody *staticBody = cpSpaceGetStaticBody(space);
	floor_y = -half_h + 10.0f;
	ceiling_y = half_h - 10.0f;
	
	const cpFloat radius = 5.0f;
	const cpFloat mass = 1.0f;
	const cpFloat moment = 0.5f * mass * radius * radius;
	
	const cpFloat segment_radius = radius + 2.0f;
	
	cpShape *floorShape = cpSegmentShapeNew(staticBody, cpv(-half_w, floor_y), cpv(half_w, floor_y), segment_radius);
	cpShapeSetElasticity(floorShape, 0.8f);
	cpShapeSetFriction(floorShape, 0.1f);
	cpSpaceAddShape(space, floorShape);
	
	cpShape *leftWallShape = cpSegmentShapeNew(staticBody, cpv(-half_w, floor_y), cpv(-half_w, ceiling_y), segment_radius);
	cpShapeSetElasticity(leftWallShape, 0.8f);
	cpShapeSetFriction(leftWallShape, 0.1f);
	cpSpaceAddShape(space, leftWallShape);
	
	cpShape *rightWallShape = cpSegmentShapeNew(staticBody, cpv(half_w, floor_y), cpv(half_w, ceiling_y), segment_radius);
	cpShapeSetElasticity(rightWallShape, 0.8f);
	cpShapeSetFriction(rightWallShape, 0.1f);
	cpSpaceAddShape(space, rightWallShape);
	
	srand((unsigned int)time(NULL));
	
	particle_count = 10;
	for(int i = 0; i < particle_count; i++){
		cpBody *body = cpBodyNew(mass, moment);
		cpFloat margin = radius + 5.0f;
		cpFloat x_min = -half_w + margin;
		cpFloat x_max = half_w - margin;
		cpFloat x = x_min + ((cpFloat)rand() / (cpFloat)RAND_MAX) * (x_max - x_min);
		cpFloat y = half_h - 50.0f + ((cpFloat)rand() / (cpFloat)RAND_MAX * 100.0f);
		cpBodySetPosition(body, cpv(x, y));
		
		cpShape *shape = cpCircleShapeNew(body, radius, cpvzero);
		cpShapeSetElasticity(shape, 0.9f);
		cpShapeSetFriction(shape, 0.1f);
		
		cpSpaceAddBody(space, body);
		cpSpaceAddShape(space, shape);
		
		particles[i].body = body;
		particles[i].shape = shape;
		particles[i].radius = radius;
	}
	
	step_count = 0;
	
	return space;
}

static void
destroy(cpSpace *space)
{
	for(int i = 0; i < particle_count; i++){
		if(particles[i].shape){
			cpSpaceRemoveShape(space, particles[i].shape);
			cpShapeFree(particles[i].shape);
		}
		if(particles[i].body){
			cpSpaceRemoveBody(space, particles[i].body);
			cpBodyFree(particles[i].body);
		}
	}
	
	particle_count = 0;
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo SleepFreezeTest = {
	"Sleep Freeze Test",
	1.0/60.0, // 60 FPS timestep
	init,
	update,
	ChipmunkDemoDefaultDrawImpl,
	destroy,
};

