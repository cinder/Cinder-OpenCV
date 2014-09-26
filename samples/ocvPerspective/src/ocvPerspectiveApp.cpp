#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// We'll create a new Cinder Application by deriving from the AppBasic class
class ocvPerspectiveApp : public AppNative {
  public:
	void	prepareSettings( Settings *settings );
	void	setup();

	int		findNearestPt( const vec2 &aPt, float minDistance );
	void	mouseDown( MouseEvent event );
	void	mouseDrag( MouseEvent event );
	void	mouseUp( MouseEvent event );

	void	updateImage();
	void	draw();

	Surface8u			mInputImage;
	vec2				mPoints[4];
	int					mTrackedPoint;
	gl::TextureRef		mTexture;
};

void ocvPerspectiveApp::prepareSettings( Settings *settings )
{
	settings->enableMultiTouch( false );
}

void ocvPerspectiveApp::setup()
{
	// The included image is copyright Pedro Szekely
	// http://www.flickr.com/photos/pedrosz/3411746271/

	mInputImage = ci::Surface8u( loadImage( loadAsset( "LAX.jpg" ) ) );	
	
	mTrackedPoint = -1;
	mPoints[0] = vec2( 150, 110 );
	mPoints[1] = vec2( getWindowWidth() - 50, 0 );
	mPoints[2] = vec2( getWindowWidth() - 20, getWindowHeight() - 10 );
	mPoints[3] = vec2( 0, getWindowHeight() - 40 );
	
	updateImage();
}

int ocvPerspectiveApp::findNearestPt( const vec2 &aPt, float minDistance )
{
	int result = -1;
	float nearestDist;
	for( size_t i = 0; i < 4; ++i ) {
		float dist = distance( mPoints[i], aPt );
		if( dist < minDistance ) {
			if( ( result == -1 ) || ( dist < nearestDist ) ) {
				result = i;
				nearestDist = dist;
			}
		}
	}
	
	return result;
}

void ocvPerspectiveApp::mouseDown( MouseEvent event )
{
	mTrackedPoint = findNearestPt( event.getPos(), 50 );
}

void ocvPerspectiveApp::mouseDrag( MouseEvent event )
{
	if( mTrackedPoint >= 0 ) {
		mPoints[mTrackedPoint] = event.getPos();
		updateImage();
	}
}

void ocvPerspectiveApp::mouseUp( MouseEvent event )
{
	mTrackedPoint = -1;
}

void ocvPerspectiveApp::updateImage()
{
	cv::Mat input( toOcv( mInputImage ) ), output;

	cv::Point2f src[4];
	src[0] = cv::Point2f( 0, 0 );
	src[1] = cv::Point2f( mInputImage.getWidth(), 0 );
	src[2] = cv::Point2f( mInputImage.getWidth(), mInputImage.getHeight() );
	src[3] = cv::Point2f( 0, mInputImage.getHeight() );
	
	cv::Point2f dst[4];
	for( int i = 0; i < 4; ++i )
		dst[i] = toOcv( mPoints[i] );
	
	cv::Mat warpMatrix = cv::getPerspectiveTransform( src, dst );
	cv::warpPerspective( input, output, warpMatrix, toOcv( getWindowSize() ), cv::INTER_CUBIC );

	mTexture = gl::Texture::create( fromOcv( output ) );
}

void ocvPerspectiveApp::draw()
{
	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );

	gl::color( Color::white() );
	gl::draw( mTexture );

	gl::color( 1.0f, 0.5f, 0.25f );
	for( int i = 0; i < 4; ++i )
		gl::drawSolidCircle( mPoints[i], 4.0f );
}

// This line tells Cinder to actually create the application
CINDER_APP_NATIVE( ocvPerspectiveApp, RendererGl )
