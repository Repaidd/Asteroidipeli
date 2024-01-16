#include "SDL.h"
#include <vector>
#include <iostream>
#include <sstream>

#define FPS 60

// m‰‰rit‰ n‰ytˆn leveys ja korkeus
#define leveys 1000
#define korkeus 800

// m‰‰ritt‰‰ objektien et‰isyyksi‰
#define pelaajanKoko leveys / 80

// m‰‰rit‰ pelaajan asetuksia
#define maksimiNopeus 4
#define kiihtyvyys 0.2
#define ampumisTiheys 30
#define ammuksenNopeus 20

// m‰‰rit‰ asteroidien ominaisuuksia
#define asteroidinKoko leveys / 20
#define pyˆrimisnopeus 0.01
#define asteroidienEnimm‰isM‰‰r‰ 7

// m‰‰rit‰ ikkuna ja renderi
SDL_Window* ikkuna = NULL;
SDL_Renderer* renderi = NULL;

// aseta syntyvien asteroidien m‰‰r‰
int asteroidienM‰‰r‰ = 1;

// pelaajan pisteet
int pisteet = 0;

// pelaajan liikuttaminen
// 0 = jarruta, 1 = kaasuta, 2 = k‰‰nny oikealle, 3 = k‰‰nny vasemmalle, 4 = ammu
bool napit[5] = {false};

bool resetoi = false;

// Avaruusobjektien yhteinen ryhm‰
struct AvaruusObjekti {

	// asemat
	float x;
	float y;

	// liikenopeus
	float dx;
	float dy;

	float kulma;

	// liikuta objektia muuttujien mukaisesti
	void liiku() {
		x += dx;
		y += dy;
	}

	void korjaaAsento() {

		if (x > leveys) {
			int ix = x;
			ix %= leveys;
			x = ix;
		}
		if (y > korkeus) {
			int iy = y;
			iy %= korkeus;
			y = iy;
		}

		if (x < 0) {
			x = leveys - 1;
		}
		if (y < 0) {
			y = korkeus - 1;
		}
	}

	// kerro objektin x ja y asemat
	void kerroAsema() {
		std::cout << x << " " << y << std::endl;
	}

};

// asteroidien luokka
struct Asteroidi : public AvaruusObjekti {

	float px[9], py[9];
	SDL_Point pisteet[9];
	char pyˆrimisSuunta;

	// asteroidin taso/koko
	int taso;

	Asteroidi( int xpos, int ypos, int ataso) {
		x = xpos;
		y = ypos;

		// satunnainen numero
		float sx = ((rand() % 5) / float(5)) + 0.1;
		float sy = ((rand() % 5) / float(5)) + 0.1;

		if (rand() % 2 == 0) {
			dx = sx;
		}
		else {
			dx = -sx;
		}
		if (rand() % 2 == 0) {
			dy = sy;
		}
		else {
			dy = -sy;
		}

		taso = ataso;

		kulma = 0;

		pyˆrimisSuunta = 'o';

		// m‰‰rittele asteroidin kulmat
		px[0] = -((asteroidinKoko / taso) / 2); px[1] = (asteroidinKoko / taso) / 2;
		px[2] = (asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2); px[3] = (asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2);
		px[4] = (asteroidinKoko / taso) / 2; px[5] = -((asteroidinKoko / taso) / 2);
		px[6] = -((asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2)); px[7] = -((asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2));
		px[8] = -((asteroidinKoko / taso) / 2);

		py[0] = (asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2); py[1] = (asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2);
		py[2] = (asteroidinKoko / taso) / 2; py[3] = -((asteroidinKoko / taso) / 2);
		py[4] = -((asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2)); py[5] = -((asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2));
		py[6] = -((asteroidinKoko / taso) / 2); py[7] = (asteroidinKoko / taso) / 2;
		py[8] = (asteroidinKoko / taso) / 2 + (asteroidinKoko / taso) / sqrt(2);

		// aseta tiedot point-listaan
		for (int i = 0; i < 9; i++) {
			pisteet[i] = { int(x) + int(px[i]), int(x + py[i]) };
		}

	}

	void pyˆri() {
		if (pyˆrimisSuunta == 'o') {
			kulma += pyˆrimisnopeus;
		}
		if (pyˆrimisSuunta == 'v') {
			kulma -= pyˆrimisnopeus;
		}

		// p‰ivit‰ asteroidin pisteet
		for (int i = 0; i < 9; i++) {
			pisteet[i] =
			{ int((px[i] * cosf(kulma) - py[i] * sinf(kulma)) + x),
			int((px[i] * sinf(kulma) + py[i] * cosf(kulma)) + y) };
		}
	}

	// reagoin ammuksen osuman kanssa
	void osuma(std::vector <Asteroidi*>& asteroidit, int tunnus) {
		
		if (taso == 1) {
			asteroidit.push_back(new Asteroidi(x - asteroidinKoko / 2, y - asteroidinKoko / 2, 2));
			asteroidit.push_back(new Asteroidi(x + asteroidinKoko / 2, y + asteroidinKoko / 2, 2));
		}
		if (taso == 2) {
			asteroidit.push_back(new Asteroidi(x + asteroidinKoko / 3, y - asteroidinKoko / 3, 3));
			asteroidit.push_back(new Asteroidi(x - asteroidinKoko / 3, y + asteroidinKoko / 3, 3));
		}
		
		asteroidit.erase(asteroidit.begin() + tunnus);
		delete this;

	}

};

// asteroidien lista
std::vector <Asteroidi*> asteroidit;

// ammusten luokka
struct Ammus : public AvaruusObjekti {

	// a = aloitusarvot

	// kasaaja
	Ammus(int xpos, int ypos, float akulma) {
		x = xpos;
		y = ypos;

		kulma = akulma;

		dx = -sin(kulma) * ammuksenNopeus;
		dy = cos(kulma) * ammuksenNopeus;
	}

	// kokeile, onko ammus yli rajojen
	bool kokeilePoistuminen(std::vector <Ammus*>& ammukset, int tunnus) {
		if (x < 0 || y < 0 || x > leveys || y > korkeus) {
			ammukset.erase(ammukset.begin() + tunnus);
			delete this;
			return true;
		}
		return false;
	}

	void tunnistaAsteroidi(std::vector <Ammus*>& ammukset, int tunnus) {
		for (int i = 0; i < asteroidit.size(); i++) {

			// ammuksen ja asteroidin v‰liset et‰isyydet
			float e[2] = { abs(x - asteroidit[i]->x), abs(y - asteroidit[i]->y)};

			// arvoista muodostuva hypotenuusa
			float hypotenuusa = sqrt(pow(e[0], 2) + pow(e[1], 2));

			if (hypotenuusa < (asteroidinKoko / asteroidit[i]->taso) / 2 + (asteroidinKoko / asteroidit[i]->taso) / sqrt(2)) {
				
				asteroidit[i]->osuma(asteroidit, i);

				ammukset.erase(ammukset.begin() + tunnus);
				delete this;
				break;
			}

		}
	}

};

// luo ammusten lista
std::vector <Ammus*> ammukset;

// Pelaaja
struct Pelaaja : public AvaruusObjekti {

	// kuvion kulmien/pisteiden muuttujat
	float px[4], py[4];

	// rajoita, kuinka usein pelajaa voi ampua
	int ajastin = 0;

	SDL_Point pisteet[4];

	Pelaaja() : AvaruusObjekti() {

		x = leveys / 2;
		y = korkeus / 2;

		px[0] = -pelaajanKoko; px[1] = pelaajanKoko; px[2] = 0; px[3] = -pelaajanKoko;
		py[0] = -pelaajanKoko; py[1] = -pelaajanKoko; py[2] = pelaajanKoko; py[3] = -pelaajanKoko;

		// muuta pisteiden arvot pisteiksi
		for (int i = 0; i < 4; i++) {
			pisteet[i] = { int(px[i]), int(py[i]) };
		}
	}

	// k‰‰nn‰ pelaajaa
	void k‰‰nn‰() {
		if (napit[2] && !napit[3]) {
			kulma += 0.1;
		}
		if (!napit[2] && napit[3]) {
			kulma -= 0.1;
		}
	}

	// kiihdyt‰ pelaajaa eteenp‰in ja hidasta kiihtyvyytt‰
	void kiihdyt‰() {
		if (napit[0]) {
			if (dx > 0) {
				dx -= 0.05;
			}
			if (dx < 0) {
				dx += 0.05;
			}
			if (dy > 0) {
				dy -= 0.05;
			}
			if (dy < 0) {
				dy += 0.05;
			}
		}
		else if (napit[1]) {
			dx += -sinf(kulma) * kiihtyvyys;
			dy += cosf(kulma) * kiihtyvyys;
		}

		// maksimi nopeus

		if (dx > maksimiNopeus) {
			dx = maksimiNopeus;
		}
		if (dx < -maksimiNopeus) {
			dx = -maksimiNopeus;
		}
		if (dy > maksimiNopeus) {
			dy = maksimiNopeus;
		}
		if (dy < -maksimiNopeus) {
			dy = -maksimiNopeus;
		}
	}

	void ammu() {
		if (ajastin > 0) {
			ajastin--;
		}

		if (ajastin == 0 && napit[4]) {
			ammukset.push_back(new Ammus(x, y, kulma));
			ajastin = ampumisTiheys;
		}
	}

	void tunnistaKosketus() {

		for (Asteroidi* asteroidi : asteroidit) {

			// pelaajan ja asteroidin v‰liset et‰isyydet
			float e[2] = { abs(x - asteroidi->x), abs(y - asteroidi->y) };

			// arvoista muodostuva hypotenuusa
			float hypotenuusa = sqrt(pow(e[0], 2) + pow(e[1], 2));

			if (hypotenuusa < (asteroidinKoko / asteroidi->taso) / 2 + (asteroidinKoko / asteroidi->taso) / sqrt(2)) {
				resetoi = true;
				break;
			}

		}

	}

};

// luo pelaaja
Pelaaja pelaaja;

// p‰ivit‰ objektien ominaisuudet
void p‰ivit‰Objektit() {

	// p‰ivit‰ pelaaja

	pelaaja.k‰‰nn‰();
	pelaaja.kiihdyt‰();
	pelaaja.liiku();
	pelaaja.korjaaAsento();
	pelaaja.ammu();
	pelaaja.tunnistaKosketus();

	// p‰ivit‰ pelaajan pisteet
	for (int i = 0; i < 4; i++) {
		pelaaja.pisteet[i] =
		{ int((pelaaja.px[i] * cosf(pelaaja.kulma) - pelaaja.py[i] * sinf(pelaaja.kulma)) + pelaaja.x),
		int((pelaaja.px[i] * sinf(pelaaja.kulma) + pelaaja.py[i] * cosf(pelaaja.kulma)) + pelaaja.y) };
	}

	// p‰ivit‰ ammukset
	for (int i = 0; i < ammukset.size(); i++) {
		ammukset[i]->liiku();

		// kokeile kosketusta asteroidiin, jos ammus ei ole yli reunojen
		if (!ammukset[i]->kokeilePoistuminen(ammukset, i)) {
			ammukset[i]->tunnistaAsteroidi(ammukset, i);
		}
	}

	// luo uusia asteroideja, jos kentt‰ on tyhj‰
	if (!asteroidit.size()) {
		for (int i = 0; i < asteroidienM‰‰r‰; i++) {
			asteroidit.push_back(new Asteroidi(rand() % leveys, rand() % korkeus, rand() % 3 + 1));
		}
		if (asteroidienM‰‰r‰ < asteroidienEnimm‰isM‰‰r‰) {
			asteroidienM‰‰r‰++;
		}
	}

	// p‰ivit‰ asteroidit
	for (Asteroidi* asteroidi : asteroidit) {
		asteroidi->pyˆri();
		asteroidi->liiku();
		asteroidi->korjaaAsento();
	}

}

// renderˆi n‰yttˆ ruudulle
void renderˆiN‰yttˆ() {
	// puhdista renderi
	SDL_SetRenderDrawColor(renderi, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderi);

	// piirr‰ pelaaja
	SDL_SetRenderDrawColor(renderi, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLines(renderi, pelaaja.pisteet, 4);

	// piirr‰ asteroidit
	for (Asteroidi* asteroidi : asteroidit) {
		SDL_RenderDrawLines(renderi, asteroidi->pisteet, 9);
	}

	// piirr‰ ammukset
	for (Ammus* ammus : ammukset) {
		SDL_RenderDrawPoint(renderi, ammus->x, ammus->y);
	}

	SDL_RenderPresent(renderi);
}

// aloita peli uudelleen
void nollaa() {
	pelaaja.x = leveys / 2; pelaaja.y = korkeus / 2; pelaaja.dx = 0; pelaaja.dy = 0;
	pelaaja.kulma = 0;
	pisteet = 0;

	asteroidit.clear();

	resetoi = false;
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	// luo ikkuna ja renderi
	ikkuna = SDL_CreateWindow("Asteroidit", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, leveys, korkeus, NULL);
	renderi = SDL_CreateRenderer(ikkuna, -1, 0);

	// luo p‰‰loopin muuttujat
	bool keskeytetty = false;
	SDL_Event event;

	// p‰‰looppi
	while (!keskeytetty) {

		// k‰y l‰pi kaikki tapahtumat
		while (SDL_PollEvent(&event)) {

			// jos rastia painetaan, lopeta ohjelma
			if (event.type == SDL_QUIT) {
				keskeytetty = true;
			}
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_DOWN:
					napit[0] = true;
					break;
				case SDLK_UP:
					napit[1] = true;
					break;
				case SDLK_RIGHT:
					napit[2] = true;
					break;
				case SDLK_LEFT:
					napit[3] = true;
					break;
				case SDLK_SPACE:
					napit[4] = true;
					break;
				}
			}
			if (event.type == SDL_KEYUP) {
				switch (event.key.keysym.sym) {
				case SDLK_DOWN:
					napit[0] = false;
					break;
				case SDLK_UP:
					napit[1] = false;
					break;
				case SDLK_RIGHT:
					napit[2] = false;
					break;
				case SDLK_LEFT:
					napit[3] = false;
					break;
				case SDLK_SPACE:
					napit[4] = false;
					break;
				}
			}
		}

		if (resetoi) {
			nollaa();
		}

		p‰ivit‰Objektit();
		renderˆiN‰yttˆ();

		SDL_Delay(1000 / FPS);

	}

	// tuhoa ikkuna ja lopeta ohjelma
	SDL_DestroyWindow(ikkuna);
	SDL_Quit();

	return 0;
}