#pragma once

class Color {
private:
	// easy reinterpret.
	union {
		struct { 
			uint8_t m_r; 
			uint8_t m_g; 
			uint8_t m_b; 
			uint8_t m_a; 
		};

		uint32_t m_rgba;
	};

public:
	// ctors.
	__forceinline Color( ) : m_r{ 0 }, m_g{ 0 }, m_b{ 0 }, m_a{ 0 }, m_rgba{} {}
	__forceinline Color( int r, int g, int b, int a = 255 ) : m_r{ ( uint8_t )r }, m_g{ ( uint8_t )g }, m_b{ ( uint8_t )b }, m_a{ ( uint8_t )a } {}
	__forceinline Color( uint32_t rgba ) : m_rgba{ rgba } {}

	static Color hsl_to_rgb( float h, float s, float l ) {
		float q;

		if( l < 0.5f )
			q = l * ( s + 1.f );

		else
			q = l + s - ( l * s );

		float p = 2 * l - q;

		float rgb[ 3 ];
		rgb[ 0 ] = h + ( 1.f / 3.f );
		rgb[ 1 ] = h;
		rgb[ 2 ] = h - ( 1.f / 3.f );

		for( int i = 0; i < 3; ++i ) {
			if( rgb[ i ] < 0 )
				rgb[ i ] += 1.f;

			if( rgb[ i ] > 1 )
				rgb[ i ] -= 1.f;

			if( rgb[ i ] < ( 1.f / 6.f ) )
				rgb[ i ] = p + ( ( q - p ) * 6 * rgb[ i ] );
			else if( rgb[ i ] < 0.5f )
				rgb[ i ] = q;
			else if( rgb[ i ] < ( 2.f / 3.f ) )
				rgb[ i ] = p + ( ( q - p ) * 6 * ( ( 2.f / 3.f ) - rgb[ i ] ) );
			else
				rgb[ i ] = p;
		}

		return { 
			int( rgb[ 0 ] * 255.f ), 
			int( rgb[ 1 ] * 255.f ), 
			int( rgb[ 2 ] * 255.f ) 
		};
	}

	static Color hsv_to_rgb(float hue, float sat, float value) {
		hue *= 360.f;

		int h = int(hue) / 60;
		float f = hue / 60.f - h;
		float p = value * (1.f - sat);
		float q = value * (1.f - f * sat);
		float t = value * (1.f - (1.f - f) * sat);

		float r, g, b;
		switch (h) {
		case 0: r = value; g = t; b = p; break;
		case 1: r = q; g = value; b = p; break;
		case 2: r = p; g = value; b = t; break;
		case 3: r = p; g = q; b = value; break;
		case 4: r = t; g = p; b = value; break;
		default: r = value; g = p; b = q; break;
		}

		return {
			int(r * 255.f),
			int(g * 255.f),
			int(b * 255.f)
		};
	}

	// member accessors.
	__forceinline uint8_t& r( ) { return m_r; }
	__forceinline uint8_t& g( ) { return m_g; }
	__forceinline uint8_t& b( ) { return m_b; }
	__forceinline uint8_t& a( ) { return m_a; }
	__forceinline uint32_t& rgba( ) { return m_rgba; }

    // operators.
    __forceinline operator uint32_t() { return m_rgba; }
};

namespace colors {
	static Color white{ 255, 255, 255, 255 };
    static Color black{ 0, 0, 0, 255 };
	static Color red{ 255, 0, 0, 255 };
	static Color burgundy{ 0xff2d00b3 };
    static Color light_blue{ 95, 174, 227, 255 };
    static Color transparent_light_blue{ 95, 174, 227, 200 };
    static Color orange{ 243, 156, 18, 255 };
    static Color transparent_blue{ 0, 0, 255, 200 };
    static Color transparent_green{ 145, 220, 54, 200 };
	static Color green{ 50, 168, 76, 255 };
	static Color fam_green{ 168, 230, 69, 205 };
    static Color transparent_yellow{ 255, 255, 0, 200 };
    static Color transparent_red{ 255, 0, 0, 200 };
	static Color reso_red{ 255, 0, 0, 170 };
	static Color reso_green{ 168, 230, 69, 170 };
	static Color reso_orange{ 243, 156, 18, 170 };
}