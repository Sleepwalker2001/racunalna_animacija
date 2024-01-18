#ifdef _WIN32
#include <windows.h>             //bit ce ukljuceno ako se koriste windows
#endif

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

//*********************************************************************************
//	Pokazivac na glavni prozor i pocetna velicina.
//*********************************************************************************

GLuint window;
GLuint sub_width = 256, sub_height = 256;

//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay();
void myReshape(int width, int height);
void idle();

//klasa za tocke, objekt sadrzi koordinate
class Dot {
public:
	float x, y, z;
	Dot(float gx, float gy, float gz) {
		x = gx;
		y = gy;
		z = gz;
	}
};
//klasa poligona, objekt sadrzi pokazivace na tocke
class Poligon {
public:
	Dot * d1, * d2, * d3;
	Poligon(Dot * gd1, Dot * gd2, Dot * gd3) {
		d1 = gd1;
		d2 = gd2;
		d3 = gd3;
	}
};

//globalne varijable koristene u programu
Dot *objDots;				//tocke objekta
Poligon *poligons;			//poligoni objekta, redoslijed tocaka
Dot *BDots;					//tocke za B-splajn
Dot *splines;				//tocke B-splajna
Dot *tangents;				//tocke za tangente B-splajna
Dot s(0.0, 0.0, 1.0);		//pocetna orijentacija
Dot e(0.0, 0.0, 0.0);		//ciljana orijentacija
Dot os(0.0, 0.0, 0.0);		//os rotacije
Dot center(0.0, 0.0, 0.0);	//srediste objekta
	
int brD;					//broj tocaka objekta
int brP;					//broj poligona
int brBD;					//broj tocaka za B-splajn
int brS;					//broj segmenata




int main(int argc, char ** argv) {

	//1. korak: ucitavanje objekta

	//provjereni objekti
	ifstream objStream("D:\\FER\\cudo\\tetrahedron.obj");
	//ifstream objStream("D:\\FER\\cudo\\kocka.obj");
	//ifstream objStream("D:\\FER\\cudo\\teddy.obj");
	vector<string> text;
	string holder;

	while (getline(objStream, holder)) {
		text.push_back(holder);
	}

	//prebroji vrhove i poligone
	for (int i = 0; i < text.size(); i++) {
		if (text.at(i).at(0) == 'v') brD++;
		else if (text.at(i).at(0) == 'f') brP++;
	}

	//alociraj potrebnu memoriju
	objDots = (Dot*)malloc(brD * sizeof(Dot));
	poligons = (Poligon*)malloc(brP * sizeof(Poligon));


	int dotCounter = 0;
	int poliCounter = 0;

	//zapisi svaki vrh i poligon zasebno
	for (int i = 0;i < text.size();i++) {
		if (text.at(i).at(0) == 'v') {
			float x, y, z;
			char *line = new char[text.at(i).size() + 1];
			std::copy(text.at(i).begin(), text.at(i).end(), line);
			sscanf(line, "v %f %f %f", &x, &y, &z);
			x *= 4;y *= 4;z *= 4;
			center.x += x;
			center.y += y;
			center.z += z;
			Dot dot(x, y, z);
			objDots[dotCounter] = dot;
			dotCounter++;
		}
		else if (text.at(i).at(0) == 'f') {
			int d1, d2, d3;
			char* line = new char[text.at(i).size() + 1];
			std::copy(text.at(i).begin(), text.at(i).end(), line);
			sscanf(line, "f %d %d %d", &d1, &d2, &d3);
			Poligon poli(&objDots[d1-1],&objDots[d2-1], &objDots[d3-1]);
			poligons[poliCounter] = poli;
			poliCounter++;
			
		}
	}
	//podijeliti srediste tijela s brojem tocaka kako bi dobili pravo srediste
	center.x /= brD;
	center.y /= brD;
	center.z /= brD;

	//2. korak ucitati niz tocaka koje odreduju aproksimacijsku uniformnu kubnu B-splajn krivulju:

	//put do bSpline datoteke
	ifstream splineStream("D:\\FER\\cudo\\bSpline.txt");
	text.clear();

	//ucitaj liniju po liniju
	while (getline(splineStream, holder)) {
		text.push_back(holder);
	}

	//zapamti broj tocaka splinea i broj segmenata i alokacija potrebne memorije
	brBD = text.size();
	brS = brBD - 4-1;
	BDots = (Dot*)malloc(brBD * sizeof(Dot));

	//spremanje pojedinih tocaka B-splinea
	for (int i = 0; i < brBD; i++) {
		float x, y, z;
		char* line = new char[text.at(i).size() + 1];
		std::copy(text.at(i).begin(), text.at(i).end(), line);
		sscanf(line, "%f %f %f", &x, &y, &z);
		Dot dot(x, y, z);
		BDots[i] = dot;
	}

	//alokacija potrebne memorije
	splines = (Dot*)malloc(100 * brS * sizeof(Dot));
	tangents = (Dot*)malloc(100 * brS * sizeof(Dot));
	int brTan = 0;
	
	//3. korak za svaki segment krivulje mijenjati parametar t od 0 do 1:
	//u svakom segmentu
	for (int i = 0;i < brS;i++) {
		Dot b1 = BDots[i];
		Dot b2 = BDots[i+1];
		Dot b3 = BDots[i+2];
		Dot b4 = BDots[i+3];

		//mijenjamo parametar t od 0 do 1:
		for (int j = 0;j < 100;j++) {
			double t = j / 100.0;

			//po uzoru na jednadzbu (1.2) iz laboratorijske vjezbe
			float f1 = (-pow(t, 3.0) + 3 * pow(t, 2.0) - 3 * t + 1) / 6.0;
			float f2 = (3 * pow(t, 3.0) - 6 * pow(t, 2.0) + 4) / 6.0;
			float f3 = (-3 * pow(t, 3.0) + 3 * pow(t, 2.0) + 3 * t + 1) / 6.0;
			float f4 = pow(t, 3.0) / 6.0;

			//po uzoru na jednadzbu (1.4) iz laboratorijske vjezbe
			float t1 = 0.5 * (-pow(t, 2.0) + 2 * t - 1) ;
			float t2 = 0.5 * (3 * pow(t, 2.0) - 4 * t) ;
			float t3 = 0.5 * (-3 * pow(t, 2.0) + 2 * t + 1) ;
			float t4 = 0.5 * (pow(t, 2.0)) ;

			//tocke krivulje
			splines[100 * i + j].x = f1 * b1.x + f2 * b2.x + f3 * b3.x + f4 * b4.x;
			splines[100 * i + j].y = f1 * b1.y + f2 * b2.y + f3 * b3.y + f4 * b4.y;
			splines[100 * i + j].z = f1 * b1.z + f2 * b2.z + f3 * b3.z + f4 * b4.z;

			//dobivamo vektore tangenti, ali ne i same tangente
			float vx = t1 * b1.x + t2 * b2.x + t3 * b3.x + t4 * b4.x;
			float vy = t1 * b1.y + t2 * b2.y + t3 * b3.y + t4 * b4.y;
			float vz = t1 * b1.z + t2 * b2.z + t3 * b3.z + t4 * b4.z;

			tangents[100 * i + j].x = splines[100 * i + j].x + vx / 2.0; //dijeljenje uvedeno radi smanjenja tangenti
			tangents[100 * i + j].y = splines[100 * i + j].y + vy / 2.0; //dijeljenje uvedeno radi smanjenja tangenti
			tangents[100 * i + j].z = splines[100 * i + j].z + vz / 2.0; //dijeljenje uvedeno radi smanjenja tangenti
		}
	}

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(sub_width, sub_height);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);

	window = glutCreateWindow("1. Pracenje putanje");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
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

//t pomocu kojeg cemo prolaziti po krivulji
int t = 0;

void myDisplay() {

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTranslatef(-5.0, -5.0, -75.0);

	//crtanje nase krivulje 
	glBegin(GL_LINE_STRIP);
	for (int i = 0;i < 100 * brS;i++) {
		glVertex3f(splines[i].x, splines[i].y, splines[i].z);
	}
	glEnd();

	//crtanje nekih od tangenta na nasu krivulju
	glBegin(GL_LINES);
	for (int i = 0;i < 100 * brS;i+=25) {
		glVertex3f(splines[i].x, splines[i].y, splines[i].z);
		glVertex3f(tangents[i].x, tangents[i].y, tangents[i].z);
	}
	glEnd();

	glTranslatef(splines[t].x, splines[t].y, splines[t].z);

	//zapisujemo rotaciju koju zelimo postici s vektorom e koji dobijemo preko tocaka krivulje i tangente na njih
	e.x = tangents[t].x - splines[t].x;
	e.y = tangents[t].y - splines[t].y;
	e.z = tangents[t].z - splines[t].z;

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
	double kut = acos(division);
	//prebacivanje u stupnjeve
	kut = kut / (2 * pi) * 360;
	//rotacija tijela
	glRotatef(kut, os.x, os.y, os.z);

	glTranslatef(-center.x, -center.y, -center.z);

	//crtanje objekta
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < brP; i++) {
		Dot p1 = *poligons[i].d1;
		Dot p2 = *poligons[i].d2;
		Dot p3 = *poligons[i].d3;

		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);
	}
	glEnd();

	//htio bih vidjeti osi objektovog vlastitog sustava
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(center.x, center.y, center.z);
	glVertex3f(center.x + 5, center.y, center.z);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(center.x, center.y, center.z);
	glVertex3f(center.x, center.y + 5, center.z);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(center.x, center.y, center.z);
	glVertex3f(center.x, center.y, center.z + 5);

	glColor3f(0.0, 0.0, 0.0);
	glEnd();

	t++;
	//nakon što prodem cijelu krivulju resetat se u sredinu
	if (t == 100 * brS) t = 0;

	glFlush();
}

int now = 0; 
int past = 0;

void idle() {
	now = glutGet(GLUT_ELAPSED_TIME);
	if ((now - past) > 10) {
		myDisplay();
		past = now;
	}
}