// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
Vec3d RayTracer::trace( double x, double y )
{
	// Clear out the ray cache in the scene for debugging purposes,
	scene->intersectCache.clear();

    ray r( Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY );

    scene->getCamera().rayThrough( x,y,r );
	Vec3d ret = traceRay( r, Vec3d(1.0,1.0,1.0), 0 );
	ret.clamp();

	if (traceUI->getAntialiasing()) {						//**** �ǉ��ӏ�(�I�v�V����) ****//

		double gridsize = 0.5; //�X�[�p�[�T���v�����O����̈�T�C�Y�i1��f=1�j

		double x_right = x + gridsize / double(buffer_width);
		if (x_right > 1) x_right = 1;
		double x_left = x - gridsize / double(buffer_width);
		if (x_left < 0) x_left = 0;
		double y_up = y + gridsize / double(buffer_height);
		if (y_up > 1) y_up = 1;
		double y_down = y - gridsize / double(buffer_height);
		if (y_down < 0) y_down = 0;

		//�E��ւ̃��C
		ray AA1(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x_right, y_up, AA1);
		Vec3d retAA1 = traceRay(AA1, Vec3d(1.0, 1.0, 1.0), 0);
		retAA1.clamp();

		//�E��
		ray AA2(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x_right, y_down, AA2);
		Vec3d retAA2 = traceRay(AA2, Vec3d(1.0, 1.0, 1.0), 0);
		retAA2.clamp();

		//����
		ray AA3(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x_left, y_up, AA3);
		Vec3d retAA3 = traceRay(AA3, Vec3d(1.0, 1.0, 1.0), 0);
		retAA3.clamp();

		//����
		ray AA4(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x_left, y_down, AA4);
		Vec3d retAA4 = traceRay(AA4, Vec3d(1.0, 1.0, 1.0), 0);
		retAA4.clamp();

		//�E
		ray AA5(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x_right, y, AA5);
		Vec3d retAA5 = traceRay(AA5, Vec3d(1.0, 1.0, 1.0), 0);
		retAA5.clamp();

		//��
		ray AA6(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x_left, y, AA6);
		Vec3d retAA6 = traceRay(AA6, Vec3d(1.0, 1.0, 1.0), 0);
		retAA6.clamp();

		//��
		ray AA7(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x, y_up, AA7);
		Vec3d retAA7 = traceRay(AA7, Vec3d(1.0, 1.0, 1.0), 0);
		retAA7.clamp();

		//��
		ray AA8(Vec3d(0, 0, 0), Vec3d(0, 0, 0), ray::VISIBILITY);
		scene->getCamera().rayThrough(x, y_down, AA8);
		Vec3d retAA8 = traceRay(AA8, Vec3d(1.0, 1.0, 1.0), 0);
		retAA8.clamp();

		ret = (ret + retAA1 + retAA2 + retAA3 + retAA4 + retAA5 + retAA6 + retAA7 + retAA8) / 9;//���ς��Ƃ�
	}
	return ret;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay( const ray& r, const Vec3d& thresh, int depth )
{
	isect i;

	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.
/*
		const Material& m = i.getMaterial();
		return m.shade(scene, r, i);

	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return Vec3d( 0.0, 0.0, 0.0 );
	}
*/
																				//**** �ǉ��ӏ�(��3��) ****//
		const Material& m = i.getMaterial();	// �����������̂̃p�����[�^�擾�p
		Vec3d ret = m.shade(scene, r, i);	// �Ԃ�l�̏����l�ɃV�F�[�f�B���O�̒l������

		if (depth < traceUI->getDepth()) {	// �ċA�[�x�ƃX���C�_�[�̒l���r
			Vec3d kr = m.kr(i);				// ���˂̌W��
			Vec3d kt = m.kt(i);				// ���߂̌W��
			Vec3d d = r.getDirection();
			d.normalize();										//**** �ǉ� ****//
			Vec3d norm = i.N;

			double n1, n2;   //���ܗ� n1 ���ˑ�, n2���ܑ�		//**** �ǉ� ****//
			if (d * i.N > 0.0) {
				norm = -norm;                          // ���̓������Ray�̏ꍇ
				n1 = m.index(i);								//**** �ǉ� ****//
				n2 = 1;
			}
			else {
				n1 = 1;
				n2 = m.index(i);
			}
			double nm = n2 / n1; 				//�S���ˏ����p

			// reflection
			Vec3d reflect(0.0, 0.0, 0.0);            // �������甽�˂��鋾�ʌ��̒l
			Vec3d dd = -(i.N*d)*i.N + d;	//�@���܂ł��g������ۑ����Ă��� d'		// Super_Hint�X���C�h���
			Vec3d rd = -d + 2 * dd; /* �O��Ɠ��l�Ɍv�Z (�����Ⴄ����)*/;
			rd.normalize();
			if (!(kr.iszero())) {                         // ���ˌW����0���`�F�b�N
				ray rr(r.at(i.t - RAY_EPSILON), rd, ray::REFLECTION);
				reflect = traceRay(rr, thresh, depth + 1);	// �[�x��[�߂čċA�v�Z
				ret += prod(kr, reflect);
			}

			// transmission
			Vec3d transmit(0.0, 0.0, 0.0);		// ���܂���Ray�̕Ԃ�l
			/* transmit�̒l�������Ōv�Z���悤. ���łɑS���˂̏ꍇ�̏��������悤 */	//�@Super_Hint�X���C�h���
			Vec3d td = -norm + dd / sqrt(nm*nm - dd.length2());  //���܃x�N�g��
			td.normalize();

			if (!kt.iszero())
			{
				if (dd.length()>nm)  //dd�̑傫����sin(��in)�Ɠ���
				{
					//�S���˂̏ꍇ��, ���˃x�N�g�����g��
					ray rr(r.at(i.t - RAY_EPSILON), rd, ray::REFLECTION);
					transmit = traceRay(rr, thresh, depth + 1);
					ret += prod(kt, transmit);
				}
				else
				{
					ray rr(r.at(i.t + RAY_EPSILON), td, ray::REFRACTION);// ���߂̏ꍇ
					transmit = traceRay(rr, thresh, depth + 1);
					ret += prod(kt, transmit);
				}
			}
			//ret += prod(kt, transmit);	//��ł��Ă邩�炢��Ȃ�
		}
		return ret;
	}
	return Vec3d(0.0, 0.0, 0.0);			// ����Ray�����̂ƌ������Ȃ������ꍇ��0
}

RayTracer::RayTracer()
	: scene( 0 ), buffer( 0 ), buffer_width( 256 ), buffer_height( 256 ), m_bBufferReady( false )
{

}


RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn )
{
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos )
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}


	if( ! sceneLoaded() )
		return false;



	// Initialize the scene's BSP tree
	scene->initBSPTree();


	
	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];


	}
	memset( buffer, 0, w*h*3 );
	m_bBufferReady = true;
}

void RayTracer::tracePixel( int i, int j )
{
	Vec3d col;

	if( ! sceneLoaded() )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);


	col = trace( x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}


