// BlobHockey.cpp : Entry point to the blob hockey madness
//

#include "BouncingBall.h"

static int WINDOW_WIDTH = 1920;
static int WINDOW_HEIGHT = 1200;

static int* FRAME_BUFFER;
static SDL_Window* WINDOW;
static SDL_Renderer* RENDERER;
static SDL_Texture* TEXTURE;
static bool DONE;

typedef struct Circle {
	unsigned int color;
	double x;
	double y;
	double r;
	double vx;
	double vy;
	double ax;
	double ay;
} Circle;

void drawBackground() {
	for (int i = 0, c = 0; i < WINDOW_HEIGHT; i++) {
		for (int j = 0; j < WINDOW_WIDTH; j++, c++) {
			FRAME_BUFFER[c] = 0xff000000;
		}
	}
}

void patchCircle(Circle* c) {
	int left = static_cast<int>(c->x - c->r);
	if (left >= WINDOW_WIDTH) {
		return;
	}
	int right = static_cast<int>(c->x + c->r);
	if (right < 0) {
		return;
	}
	int up = static_cast<int>(c->y - c->r);
	if (up >= WINDOW_HEIGHT) {
		return;
	}
	int down = static_cast<int>(c->y + c->r);
	if (down < 0) {
		return;
	}
	if (left < 0) {
		left = 0;
	}
	if (right > WINDOW_WIDTH) {
		right = WINDOW_WIDTH;
	}
	if (up < 0) {
		up = 0;
	}
	if (down > WINDOW_HEIGHT) {
		down = WINDOW_HEIGHT;
	}
	for (int i = up; i < down; i++) {
		for (int j = left; j < right; j++) {
			FRAME_BUFFER[WINDOW_WIDTH * i + j] = 0xff000000;
		}
	}
}

void drawCircle(Circle* c) {
	int rr = static_cast<int>(c->r * c->r);
	int left = static_cast<int>(c->x - c->r);
	if (left >= WINDOW_WIDTH) {
		return;
	}
	int right = static_cast<int>(c->x + c->r);
	if (right < 0) {
		return;
	}
	int up = static_cast<int>(c->y - c->r);
	if (up >= WINDOW_HEIGHT) {
		return;
	}
	int down = static_cast<int>(c->y + c->r);
	if (down < 0) {
		return;
	}
	if (left < 0) {
		left = 0;
	}
	if (right > WINDOW_WIDTH) {
		right = WINDOW_WIDTH;
	}
	if (up < 0) {
		up = 0;
	}
	if (down > WINDOW_HEIGHT) {
		down = WINDOW_HEIGHT;
	}
	int x = static_cast<int>(c->x);
	int y = static_cast<int>(c->y);
	for (int i = up; i < down; i++) {
		for (int j = left; j < right; j++) {
			int xx = j - x;
			xx *= xx;
			int yy = i - y;
			yy *= yy;
			if (xx + yy < rr) {
				FRAME_BUFFER[WINDOW_WIDTH * i + j] = c->color;
			}
		}
	}
}

void updatePhysics(Circle* c, double dt) {

	c->vx += c->ax * dt;
	c->vy += c->ay * dt;

	const double drag = 0.995;
	c->vx *= drag;
	c->vy *= drag;
	c->x += c->vx * dt;
	c->y += c->vy * dt;

	const double damping = 0.8;
	double left = c->x - c->r;
	if (left < 0) {
		c->vx = -c->vx * damping;
		c->x = c->r;
	}
	double right = c->x + c->r;
	if (right >= WINDOW_WIDTH) {
		c->vx = -c->vx * damping;
		c->x = WINDOW_WIDTH - c->r;
	}
	double up = c->y - c->r;
	if (up < 0) {
		c->vy = -c->vy * damping;
		c->y = c->r;
	}
	double down = c->y + c->r;
	if (down >= WINDOW_HEIGHT) {
		c->vy = -c->vy * damping;
		c->y = WINDOW_HEIGHT - c->r;
	}
}

void collide(Circle* c1, Circle* c2) {
	c2->color = 0xff0000ff;

	double vx = c1->vx - c2->vx;
	double vy = c1->vy - c2->vy;
	double a = vx * vx + vy * vy;

	double x = c1->x - c2->x;
	double y = c1->y - c2->y;
	double b = -2 * (x * vx + y * vy);

	double rr = c1->r + c2->r;
	rr *= rr;
	double c = x * x + y * y - rr;

	// t should always be positive. if collisions are weird, check here
	double t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

	c1->x = c1->x - c1->vx * t;
	c1->y = c1->y - c1->vy * t;

	c2->x = c2->x - c2->vx * t;
	c2->y = c2->y - c2->vy * t;

	x = c1->x - c2->x;
	y = c1->y - c2->y;

	// update velocities
	double mag = (vx * x + vy * y) / (x * x + y * y);
	c1->vx = c1->vx - mag * x;
	c1->vy = c1->vy - mag * y;
	c2->vx = c2->vx + mag * x;
	c2->vy = c2->vy + mag * y;

	// update positions
	c1->x += c1->vx * t;
	c1->y += c1->vy * t;
	c2->x += c2->vx * t;
	c2->y += c2->vy * t;
}

void checkCollisions(Circle c[], int num) {
	for (int i = 0; i < num - 1; i++) {
		for (int j = i + 1; j < num; j++) {
			double xDif = c[i].x - c[j].x;
			xDif *= xDif;
			double yDif = c[i].y - c[j].y;
			yDif *= yDif;
			double rr = c[i].r + c[j].r;
			rr *= rr;
			if (xDif + yDif <= rr) {
				collide(&c[i], &c[j]);
			}
		}
	}
}

void updateTexture1() {
	char* pix;
	int pitch;

	SDL_LockTexture(TEXTURE, NULL, (void**)&pix, &pitch);
	for (int i = 0, sp = 0, dp = 0; i < WINDOW_HEIGHT; i++, dp += WINDOW_WIDTH, sp += pitch)
		memcpy(pix + sp, FRAME_BUFFER + dp, WINDOW_WIDTH * 4);

	SDL_UnlockTexture(TEXTURE);
	SDL_RenderTexture(RENDERER, TEXTURE, NULL, NULL);
	SDL_RenderPresent(RENDERER);
}

void updateTexture2() {
	SDL_UpdateTexture(TEXTURE, NULL, FRAME_BUFFER, WINDOW_WIDTH * 4);
	SDL_RenderTexture(RENDERER, TEXTURE, NULL, NULL);
	SDL_RenderPresent(RENDERER);
}

void loop() {
	static uint64_t lastTick = 0;
	static Circle c[3] = {
		{0xffff0000, WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 4.0, WINDOW_HEIGHT / 16.0, 50.0, -100.0, 0, 9.8},
		{0xff00ff00, WINDOW_WIDTH / 2.0, WINDOW_HEIGHT - WINDOW_HEIGHT / 16.0, WINDOW_HEIGHT / 16.0, 0, 0, 0, 0},
		{0, 0, 50, 0xff0000ff, 0, 0, 0, 9.8}
	};
	SDL_Event event;
	SDL_PollEvent(&event);
	if (event.type == SDL_EVENT_QUIT) {
		DONE = true;
		return;
	}
	if (event.type == SDL_EVENT_KEY_UP && event.key.key == SDLK_ESCAPE) {
		DONE = true;
		return;
	}

	updateTexture2();

	patchCircle(c);
	patchCircle(c + 1);

	float mouseX;
	float mouseY;

	uint64_t tick = SDL_GetTicks();
	double dt = (tick - lastTick) * 0.02;
	SDL_GetGlobalMouseState(&mouseX, &mouseY);
	c[1].vx = (mouseX - c[1].x) / dt;
	c[1].vy = (mouseY - c[1].y) / dt;
	c[1].x = mouseX;
	c[1].y = mouseY;
	updatePhysics(c, dt);
	checkCollisions(c, 2);

	drawCircle(c);
	drawCircle(c + 1);
	lastTick = tick;
}

int main(int argc, char** argv) {
	
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}
	SDL_DisplayID* displays = SDL_GetDisplays(NULL);
	if (displays != NULL) {
		const SDL_DisplayMode* dMode = SDL_GetDesktopDisplayMode(displays[0]);
		WINDOW_WIDTH = dMode->w;
		WINDOW_HEIGHT = dMode->h;
	}
	FRAME_BUFFER = new int[WINDOW_WIDTH * WINDOW_HEIGHT];
	WINDOW = SDL_CreateWindow("Bouncing Ball", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (WINDOW == NULL) {
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}
	RENDERER = SDL_CreateRenderer(WINDOW, NULL);
	if (RENDERER == NULL) {
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}
	TEXTURE = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
	if (TEXTURE == NULL) {
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_SetWindowFullscreen(WINDOW, true);

	drawBackground();

	DONE = false;
	while (!DONE) {
		loop();
	}

	SDL_DestroyTexture(TEXTURE);
	SDL_DestroyRenderer(RENDERER);
	SDL_DestroyWindow(WINDOW);
	SDL_Quit();

	return 0;
}
