#pragma once

#define COLORPICKER_WIDTH		20
#define COlORPICKER_HEIGHT		8
#define COLORPICKER_PICKER_SIZE 210

class Colorpicker : public Element {
public:
	__forceinline Colorpicker() : m_open{ false }, m_label{}, m_color{}, m_ptr{ nullptr } {
		m_flags = ElementFlags::DRAW | ElementFlags::CLICK | ElementFlags::ACTIVE | ElementFlags::SAVE | ElementFlags::DEACIVATE;
		m_type = ElementTypes::COLORPICKER;
		m_h = m_base_h = COlORPICKER_HEIGHT;
		m_show = true;
	}


	__forceinline void setup(const std::string& label, const std::string& file_id, Color color, Color* ptr = nullptr, bool use_label = false) {
		m_use_label = use_label;
		m_label = label;
		m_file_id = file_id;
		m_color = color;
		m_ptr = ptr;

		if (m_ptr)
			*m_ptr = m_color;
	}

	__forceinline void set(Color color) {
		bool changed = m_color.rgba() != color.rgba();

		m_color = color;

		if (m_ptr)
			*m_ptr = m_color;

		if (changed && m_callback)
			m_callback();
	}

	__forceinline Color get() {
		return m_color;
	}

	static __forceinline Color ColorFromPos(int x, int y) {
		return *(Color*)(gradient.get() + std::clamp(x, 1, COLORPICKER_PICKER_SIZE - 1) + std::clamp(y, 1, COLORPICKER_PICKER_SIZE - 1) * COLORPICKER_PICKER_SIZE);
	}

	static __forceinline Color ColorfromAlpha(int x) {
		return *(Color*)(gradient.get() + x);
	}

public:
	static int texture;
	static std::unique_ptr< Color[] > gradient;
protected:
	float       m_hue;
	bool		m_open;
	bool		m_new_bool;
	std::string m_label;
	Color		m_color;
	bool        m_in_line;

	Color* m_ptr;

protected:
	void init();
	void draw() override;
	void think() override;
	void click() override;
};