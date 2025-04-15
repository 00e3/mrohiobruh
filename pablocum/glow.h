#pragma once

#define END_OF_FREE_LIST	-1
#define ENTRY_IN_USE		-2

struct GlowObjectDefinition_t {
	Entity* m_entity;
	vec3_t m_color;
	float m_alpha;

	bool m_alpha_capped_by_render_alpha;
	float m_glow_alpha_function_of_max_velocity;
	float m_max_alpha;
	float m_glow_pulse_override;
	bool m_render_occluded;
	bool m_render_unoccluded;
	bool m_render_full_bloom;
	int m_full_bloom_stencil_test_value; // only render full bloom objects if stencil is equal to this value (value of -1 implies no stencil test)
	int m_render_style;
	int m_split_screen_slot;

	// Linked list of free slots
	int m_next_free_slot;
};

class CGlowObjectManager {
public:
	CUtlVector< GlowObjectDefinition_t >	m_object_definitions;
	int										m_first_free_slot;
};