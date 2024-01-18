#ifdef _WIN32
#include <windows.h>             //bit ce ukljuceno ako se koriste windows
#endif

//  #include <GL/Gl.h>
//  #include <GL/Glu.h>    nije potrebno ako se koristi glut
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

//postavke za kutiju
double boxMinX = -10.0;
double boxMaxX = 10.0;
double boxMinY = -10.0;
double boxMaxY = 10.0;
double boxMinZ = -10.0;
double boxMaxZ = 10.0;

//postavke za kuglu
const double globeRadius = 10.0;

//potrebne klase
class Source {
public:
	float x, y, z;
	float r, g, b;
	int freq;
	double size;
};

class Vertex {
public:
	float x, y, z;
	Vertex(float given_x, float given_y, float given_z) {
		x = given_x, y = given_y, z = given_z;
	}
};

class Particle {
public:
	float x, y, z;
	float r, g, b;
	float vx, vy, vz;
	int life;
	float sX, sY, sZ;
	float osX, osY, osZ;
	double angle, size;
};


//potrebne varijable
Source center;
Source center2;
vector<Particle> allSnow;

int now, before;
int temp = -1;
GLuint texture;
Vertex pov(0.0, 0.0, 50.0);
int shape = 0;

//*********************************************************************************
//	Pokazivac na glavni prozor i pocetna velicina.
//*********************************************************************************

GLuint window;
GLuint sub_width = 512, sub_height = 512;

//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay();
void myReshape(int width, int height);
void myIdle();
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);

GLuint loadTexture(const char* file, int wrap);

void myParticle(Particle snow);

//prikaz
void myDisplay() {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(pov.x, pov.y, -pov.z);

	//prikazi svaku cesticu
	for (int i = 0;i < allSnow.size();i++) {
		myParticle(allSnow.at(i));
	}
	glutSwapBuffers();
}

void myParticle(Particle snow) {
	//boja
	glColor3f(snow.r, snow.g, snow.b);
	//polozaj
	glTranslatef(snow.x, snow.y, snow.z);
	//rotacija
	glRotatef(snow.angle, snow.osX, snow.osY, snow.osZ);
	//crtanje kvadrata s teksturom
	glBegin(GL_QUADS);
		
	glTexCoord2d(0.0, 0.0); glVertex3f(-snow.size, -snow.size, 0.0);
	glTexCoord2d(1.0, 0.0); glVertex3f(-snow.size, +snow.size, 0.0);
	glTexCoord2d(1.0, 1.0); glVertex3f(+snow.size, +snow.size, 0.0);
	glTexCoord2d(0.0, 1.0); glVertex3f(+snow.size, -snow.size, 0.0);

	glEnd();
	glRotatef(-snow.angle, snow.osX, snow.osY, snow.osZ);
	glTranslatef(-snow.x, -snow.y, -snow.z);
}

void myReshape(int width, int height)
{
	sub_width = width;                      	//promjena sirine prozora
	sub_height = height;						//promjena visine prozora

	glViewport(0, 0, sub_width, sub_height);	//  otvor u prozoru

	//-------------------------
	glMatrixMode(GL_PROJECTION);                // Select The Projection Matrix
	glLoadIdentity();                           // Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);                 // Select The Modelview Matrix
	glLoadIdentity();                           // Reset The Modelview Matrix

}

//dok radi
void myIdle(){
	//provjeri koliko vremena je proslo
	now = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = now - before;

	//ako je proslo dovoljno vremena
	if (elapsed > 100) {
		//ukoliko smijem stvarati jos cestica
		if (center.freq > 0) {
			//stvori ih relativno random
			int new_snow = rand() % center.freq;
			//odredi sve sto je potrebno za cesticu
			for (int i = 0;i < new_snow;i++) {
				Particle flake;
				flake.x = center.x;
				flake.z = center.z;
				flake.y = center.y;
				flake.r = 1.0;
				flake.g = 1.0;
				flake.b = 1.0;
				flake.vx = 0.5;
				flake.vy = 0.5;
				flake.vz = 0.5;
				flake.sX = (rand() % 2000 - 1000) / (double)1000;
				flake.sY = (rand() % 2000 - 1000) / (double)1000;
				flake.sZ = (rand() % 2000 - 1000) / (double)1000;
				flake.life = 150 + (rand() % 20 - 10);
				flake.size = center.size;
				allSnow.push_back(flake);
			}
			for (int i = 0;i < new_snow;i++) {
				Particle flake;
				flake.x = center2.x;
				flake.z = center2.z;
				flake.y = center2.y;
				flake.r = 1.0;
				flake.g = 0.0;
				flake.b = 0.0;
				flake.vx = 0.5;
				flake.vy = 0.5;
				flake.vz = 0.5;
				flake.sX = (rand() % 2000 - 1000) / (double)1000;
				flake.sY = (rand() % 2000 - 1000) / (double)1000;
				flake.sZ = (rand() % 2000 - 1000) / (double)1000;
				flake.life = 150 + (rand() % 20 - 10);
				flake.size = center2.size;
				allSnow.push_back(flake);
			}
		}
		//printf("%d\n", allSnow.size());
		//pomicanje svih postojecih cestica
		for (int i = allSnow.size()-1;i >= 0;i--) {
			allSnow.at(i).life--;
			if (allSnow.at(i).life <= 0) {
				allSnow.erase(allSnow.begin() + i);
			}
			else {
				Vertex s(0.0, 0.0, 1.0);
				Vertex e(0.0, 0.0, 0.0);
				Vertex os(0.0, 0.0, 0.0);

				//zapisujemo rotaciju koju zelimo postici s vektorom e koji dobijemo preko pozicije cestice i pov-a
				e.x = allSnow.at(i).x - pov.x;
				e.y = allSnow.at(i).y - pov.y;
				e.z = allSnow.at(i).z - pov.z;

				//dolazimo do osi rotacije kao vektorskog produkta pocetne rotacije i ciljane
				//jednadzba 1.5
				os.x = s.y * e.z - e.y * s.z;
				os.y = -(s.x * e.z - e.x * s.z);
				os.z = s.x * e.y - s.y * e.x;

				//kut rotacije prema jednadzbi 1.6
				float pi = 3.14159265;
				double apsS = pow(pow((double)s.x, 2.0) + pow((double)s.y, 2.0) + pow((double)s.z, 2.0), 0.5);
				double apsE = pow(pow((double)e.x, 2.0) + pow((double)e.y, 2.0) + pow((double)e.z, 2.0), 0.5);
				double se = s.x * e.x + s.y * e.y + s.z * e.z;
				double division = se / (apsS * apsE);
				//dobiveni rezultat je u radijanima
				double angle = acos(division);
				//prebacivanje u stupnjeve
				angle = angle / (2 * pi) * 360;
				//zapis u cesticu
				allSnow.at(i).osX = os.x;
				allSnow.at(i).osY = os.y;
				allSnow.at(i).osZ = os.z;
				allSnow.at(i).angle = angle;

				//promjena položaja pomocu brzine i puta
				allSnow.at(i).x += allSnow.at(i).vx * allSnow.at(i).sX;
				allSnow.at(i).y += allSnow.at(i).vy * allSnow.at(i).sY;
				allSnow.at(i).z += allSnow.at(i).vz * allSnow.at(i).sZ;

				//promjena boje kroz vrijeme
				if (allSnow.at(i).r > 0) allSnow.at(i).r -= 0.008;
				if (allSnow.at(i).b < 1.0) allSnow.at(i).b += 0.008;

				//provjera sudara li se pahuljica s kuglom
				double distanceToGlobeCenter = sqrt(
					pow(allSnow.at(i).x, 2.0) + pow(allSnow.at(i).y, 2.0) + pow(allSnow.at(i).z, 2.0)
				);

				//ako se sudari neka pahuljice padnu ravno dolje
				if (distanceToGlobeCenter > globeRadius) {

					//odbijanje i klizenje po radijusu kugle
					double scaleFactor = globeRadius / distanceToGlobeCenter;
					allSnow.at(i).x *= scaleFactor;
					allSnow.at(i).y *= scaleFactor;
					allSnow.at(i).z *= scaleFactor;

					//ravno dolje
					allSnow.at(i).sX = 0.0;
					allSnow.at(i).sY = -1.0;
					//suprotan smjer
					allSnow.at(i).sZ *= -1.0;

				}
			}
		}
		myDisplay();
		before = now;
	}
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
	if (theKey == 'w') center.y += 0.5;
	if (theKey == 's') center.y -= 0.5;
	if (theKey == 'a') center.x -= 0.5;
	if (theKey == 'd') center.x += 0.5;
	if (theKey == 'q') center.z += 0.5;
	if (theKey == 'e') center.z -= 0.5;

	if (theKey == 'm' && center.freq > 0) center.freq++;
	if (theKey == 'n' && center.freq<= 50) center.freq--;

	if (theKey == '+' && center.size < 4.0) center.size += 0.01;
	if (theKey == '-' && center.size > 0.02) center.size -= 0.01;

	if (theKey == 'u') pov.y += 0.1;
	if (theKey == 'j') pov.y -= 0.1;
	if (theKey == 'h') pov.x -= 0.1;
	if (theKey == 'k') pov.x += 0.1;
}

GLuint loadTexture(const char* filename, int wrap)
{
	GLuint texture;
	int width, height;
	BYTE* data;
	FILE* file;
	// otvori datoteku teksture
	file = fopen(filename, "rb");
	if (file == NULL) {
		return 0;
	}

	// alociraj potrebnu memoriju
	width = 256;
	height = 256;
	data = (BYTE*)malloc(width * height * 3);

	// procitaj datoteku
	fread(data, width * height * 3, 1, file);
	fclose(file);

	// postavi ime teksture
	glGenTextures(1, &texture);

	// odaberi teksturu
	glBindTexture(GL_TEXTURE_2D, texture);

	// miješanje boje i teksture
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// za male povrsine
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	// za velike povrsine
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// ako je wrap true, tekstura ce se zamotati preko rubova inace tekstura zavrsava na rubovima
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);

	// izgradi mipmap teksture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height,
		GL_RGB, GL_UNSIGNED_BYTE, data);

	// oslobodi buffer
	free(data);

	return texture;
}

int main(int argc, char** argv) {
	//postavljanje centra
	center.x = 0.0;
	center.y = 0.0;
	center.z = 0.0;
	center.freq = 5;
	center.r = 1.0;
	center.g = 1.0;
	center.b = 1.0;
	center.size = 0.4;

	center2.x = 5.0;
	center2.y = 0.0;
	center2.z = 0.0;
	center2.freq = 5;
	center2.r = 1.0;
	center2.g = 0.0;
	center2.b = 0.0;
	center2.size = 0.4;
	srand(time(NULL));
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(sub_width, sub_height);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);

	window = glutCreateWindow("2. Sustav cestica");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay); 
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(myIdle);

	//ucitavanje teksture
	texture = loadTexture("snow.bmp", 0);
	//prozirnost
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);				
	glEnable(GL_BLEND); 


	glEnable(GL_TEXTURE_2D); 
	glBindTexture(GL_TEXTURE_2D, texture); 

	glutMainLoop();
	return 0;

}