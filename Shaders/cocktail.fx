/*------------------.
| :: Description :: |
'-------------------/
	
	Cocktail

	Author: Christopher Perkins
	License: CC-BY 4.0

	About:
	Displays the image twice mirrored at the center for a coctail cab.

*/

/*---------------.
| :: Includes :: |
'---------------*/

#include "ReShade.fxh"

/*---------------.
| :: ui       :: |
'---------------*/

uniform int ratioI <
	ui_type = "combo";
	ui_label = "Ratio of Source";
	ui_tooltip = "Choose an Aspect Ratio";
	ui_items =
	"3:4\0"
	"4:3\0"
	"3:2\0"
	"16:9\0"
	;
> = 1;

uniform int ratio <
	ui_type = "combo";
	ui_label = "Ratio of Output";
	ui_tooltip = "Choose an Aspect Ratio";
	ui_items = 
	"4:3\0"
	"3:2\0"
	"16:9\0"
	;
> = 0;

uniform float4 background <
	ui_type = "color";
	ui_label= "Back";
	ui_tooltip = "Choose a color";
> = float4(0,0,0,0);

/*-------------.
| :: Effect :: |
'-------------*/

float4 PS_Cocktail(float4 vpos: SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{	
	float2 newcoord = texcoord;

	float a = 1;
	float b = 1;
	float x = 1;
	float y = 1;

	switch (ratioI) {
		case 0:
			x = 3;
			y = 4;
			break;
		case 1:
			x = 4;
			y = 3;
			break;
		case 2:
			x = 3;  // easier to do 3:2 as thats the part we are copying to
			y = 2;
			break;
		case 3:
			x = 16;
			y = 9;
			break;
	}

	switch (ratio) {
		case 0:
			a = 4;
			b = 3;
			break;
		case 1:
			a = 3;
			b = 2;
			break;
		case 2:
			a = 16;
			b = 9;
			break;
	}

	// We calculate how much of the vertical screen is taken up by the source.
	float sourceHeightPCT = (BUFFER_WIDTH/x*y)/BUFFER_HEIGHT;
	// Now we calculate how much of the screen will be taken up by each of the duplicated screens
	float destinationHeightPCT = (BUFFER_WIDTH/a*b)/BUFFER_HEIGHT;

	// We normally assume that the width will always be full accross
	float destinationWidthPCT = 1;
	// if the desired height ends up as more than half the screen, we gotta pillar box.
	if (destinationHeightPCT > .5) {
		destinationHeightPCT = .5;
		destinationWidthPCT = (BUFFER_HEIGHT/2/b*a)/BUFFER_WIDTH;
	}
	
	// where the picture starts on the Y axis
	float sourceYPos = .5 - sourceHeightPCT/2;
	// how much letterboxing do we need
	float ySpacing = (.5 - destinationHeightPCT)/2;
	// how much pillarboxing
	float xSpacing = (1-destinationWidthPCT)/2;
	
	if (texcoord.x > xSpacing && texcoord.x < 1-xSpacing) // in bounds
		newcoord.x = (texcoord.x - xSpacing)/destinationWidthPCT;
	else return background; // out of bounds of pillar box

	if (texcoord.y > ySpacing && texcoord.y < .5-ySpacing) {
		// We are at the top of the screen.
		newcoord.y = (1-(texcoord.y-ySpacing)/destinationHeightPCT) * sourceHeightPCT + sourceYPos;
		// gotta flip the x to look right
		newcoord.x = 1-newcoord.x;
	} else if (texcoord.y > .5+ySpacing && texcoord.y < 1-ySpacing) // bottom
		newcoord.y = (texcoord.y-ySpacing - .5)/destinationHeightPCT * sourceHeightPCT + sourceYPos;
	else // out of bounds
		return background;

	return tex2D(ReShade::BackBuffer,newcoord);
}


/*-----------------.
| :: Techniques :: |
'-----------------*/

technique Cocktail
{
    pass
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Cocktail;
    }
}
