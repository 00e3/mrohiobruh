#pragma once

class WeaponCfg
{
public:
	MultiDropdown hitbox;
	MultiDropdown hitbox_air;
	Slider		  air_scale;
	Slider		  scale;
	Slider		  body_scale;
	Slider		  minimal_damage;
	Checkbox	  penetrate;
	Checkbox	  minimal_damage_hp;

	MultiDropdown auto_stop;

	Slider	      hitchance_amount;
	Slider	      accuracy_boost;
	Checkbox      prefer_safety;
	MultiDropdown force_safety;
	MultiDropdown baim1;
	MultiDropdown baim2;
	Slider        baim_hp;
};


class AimbotTab : public Tab
{
public:
	// col1.
	Checkbox	  enable;
	Checkbox	  silent;
	Dropdown	  selection;
	Dropdown	  scope;
	Dropdown	  weapon;

	WeaponCfg     auto_sniper;
	WeaponCfg     awp;
	WeaponCfg     scout;
	WeaponCfg     heavy_pistol;
	WeaponCfg     pistol;
	WeaponCfg     general;

	Keybind       baim_key;
	Keybind       dmg_key;
	Slider        dmg_ov;

	Keybind       ov_correction;
	Checkbox      debug_aim_points;
	Checkbox      aim_matrix;
	Colorpicker   aim_matrix_col;
	Colorpicker   ov_correction_col;

	Keybind       increase_firerate;
	Slider        firerate;

public:
	void init( )
	{
		// title.
		SetTitle( XOR( "aimbot" ) );

		enable.setup( XOR( "enable" ), XOR( "enable" ) );
		RegisterElement( &enable );

		silent.setup( XOR( "silent aimbot" ), XOR( "silent" ) );
		RegisterElement( &silent );

		selection.setup( XOR( "target selection" ), XOR( "selection" ), { XOR( "distance" ), XOR( "crosshair" ), XOR( "damage" ), XOR( "health" ), XOR( "lag" ), XOR( "height" ) } );
		RegisterElement( &selection );

		weapon.setup( XOR( "weapon" ), XOR( "weapon" ), { XOR( "general" ), XOR( "auto" ), XOR( "awp" ), XOR( "scout" ), XOR( "heavy-pistol" ), XOR( "pistol" ) } );
		RegisterElement( &weapon );

		// general
		{
			general.hitchance_amount.setup( "hitchance", XOR( "hitchance_amount_general" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			general.hitchance_amount.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.hitchance_amount );

			general.accuracy_boost.setup( "accuracy boost", XOR( "accuracy_boost_general" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			general.accuracy_boost.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.accuracy_boost );

			general.hitbox.setup( XOR( "hitbox" ), XOR( "hitbox_general" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			general.hitbox.AddShowCallback( callbacks::IsGeneral );
			general.hitbox.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.hitbox );

			general.hitbox_air.setup( XOR( "hitbox in air" ), XOR( "hitbox_air_general" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			general.hitbox_air.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.hitbox_air );

			general.air_scale.setup( XOR( "in air scale multiplier" ), XOR( "hitbox_air_scale_general" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			general.air_scale.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.air_scale );

			general.scale.setup( XOR( "head scale" ), XOR( "hitbox_scale_general" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			general.scale.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.scale );

			general.body_scale.setup( XOR( "body scale" ), XOR( "body_hitbox_scale_general" ), 1.f, 100.f, true, 0, 50.f, 1.f, XOR( L"%" ) );
			general.body_scale.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.body_scale );

			general.minimal_damage.setup( XOR( "minimal damage" ), XOR( "minimal_damage_general" ), 1.f, 110.f, true, 0, 40.f, 1.f, XOR( L"hp" ) );
			general.minimal_damage.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.minimal_damage );

			general.penetrate.setup( XOR( "penetrate walls" ), XOR( "penetrate_general" ) );
			general.penetrate.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.penetrate );

			general.minimal_damage_hp.setup( XOR( "scale damage on hp" ), XOR( "minimal_damage_hp_general" ) );
			general.minimal_damage_hp.AddShowCallback(callbacks::IsBelowHun);
			general.minimal_damage_hp.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.minimal_damage_hp );

			general.prefer_safety.setup( XOR( "prefer safety" ), XOR( "prefer_safety_general" ) );
			general.prefer_safety.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.prefer_safety, 1 );

			general.force_safety.setup( XOR( "force safety" ), XOR( "force_safety_general" ), { XOR( "walking" ), XOR( "standing" ), XOR( "in air" ), XOR( "update" ), XOR( "potentially lethal body aim" ), XOR( "predicted landing" ) } );
			general.force_safety.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.force_safety, 1 );

			general.baim1.setup( XOR( "prefer body aim" ), XOR( "baim1_general" ), { XOR( "always" ), XOR( "in air" ), XOR( "lethal x2" ) } );
			general.baim1.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.baim1, 1 );

			general.baim2.setup( XOR( "only body aim" ), XOR( "baim2_general" ), { XOR( "always" ), XOR( "health" ), XOR( "in air" ) } );
			general.baim2.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.baim2, 1 );

			general.baim_hp.setup( "", XOR( "baim_hp_general" ), 1.f, 50.f, false, 0, 20.f, 1.f, XOR( L"hp" ) );
			general.baim_hp.AddShowCallback( callbacks::IsGeneral );
			general.baim_hp.AddShowCallback( callbacks::IsBaimHPOn );
			RegisterElement( &general.baim_hp, 1 );

			general.auto_stop.setup( XOR( "auto stop" ), XOR( "auto_stop_general" ), { XOR( "between shots" ), XOR( "in air" ) } );
			general.auto_stop.AddShowCallback( callbacks::IsGeneral );
			RegisterElement( &general.auto_stop, 1 );
		}

		// auto
		{
			auto_sniper.hitchance_amount.setup( "hitchance", XOR( "hitchance_amount_auto_sniper" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			auto_sniper.hitchance_amount.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.hitchance_amount );

			auto_sniper.accuracy_boost.setup( "accuracy boost", XOR( "accuracy_boost_auto_sniper" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			auto_sniper.accuracy_boost.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.accuracy_boost );

			auto_sniper.hitbox.setup( XOR( "hitbox" ), XOR( "hitbox_auto_sniper" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			auto_sniper.hitbox.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.hitbox );

			auto_sniper.hitbox_air.setup( XOR( "hitbox in air" ), XOR( "hitbox_air_auto_sniper" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			auto_sniper.hitbox_air.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.hitbox_air );

			auto_sniper.air_scale.setup( XOR( "in air scale multiplier" ), XOR( "hitbox_air_scale_auto_sniper" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			auto_sniper.air_scale.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.air_scale );

			auto_sniper.scale.setup( XOR( "head scale" ), XOR( "hitbox_scale_auto_sniper" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			auto_sniper.scale.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.scale );

			auto_sniper.body_scale.setup( XOR( "body scale" ), XOR( "body_hitbox_scale_auto_sniper" ), 1.f, 100.f, true, 0, 50.f, 1.f, XOR( L"%" ) );
			auto_sniper.body_scale.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.body_scale );

			auto_sniper.minimal_damage.setup( XOR( "minimal damage" ), XOR( "minimal_damage_auto_sniper" ), 1.f, 110.f, true, 0, 40.f, 1.f, XOR( L"hp" ) );
			auto_sniper.minimal_damage.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.minimal_damage );

			auto_sniper.penetrate.setup( XOR( "penetrate walls" ), XOR( "penetrate_auto_sniper" ) );
			auto_sniper.penetrate.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.penetrate );

			auto_sniper.minimal_damage_hp.setup( XOR( "scale damage on hp" ), XOR( "minimal_damage_hp_auto_sniper" ) );
			auto_sniper.minimal_damage_hp.AddShowCallback(callbacks::IsBelowHunauto);
			auto_sniper.minimal_damage_hp.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.minimal_damage_hp );

			auto_sniper.prefer_safety.setup( XOR( "prefer safety" ), XOR( "prefer_safety_auto_sniper" ) );
			auto_sniper.prefer_safety.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.prefer_safety, 1 );

			auto_sniper.force_safety.setup( XOR( "force safety" ), XOR( "force_safety_auto_sniper" ), { XOR( "walking" ), XOR( "standing" ), XOR( "in air" ), XOR( "update" ), XOR( "potentially lethal body aim" ), XOR( "predicted landing" ) } );
			auto_sniper.force_safety.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.force_safety, 1 );

			auto_sniper.baim1.setup( XOR( "prefer body aim" ), XOR( "baim1_auto_sniper" ), { XOR( "always " ), XOR( "in air" ), XOR( "lethal x2" ) } );
			auto_sniper.baim1.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.baim1, 1 );

			auto_sniper.baim2.setup( XOR( "only body aim" ), XOR( "baim2_auto_sniper" ), { XOR( "always" ), XOR( "health" ), XOR( "in air" ) } );
			auto_sniper.baim2.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.baim2, 1 );

			auto_sniper.baim_hp.setup( "", XOR( "baim_hp_auto_sniper" ), 1.f, 50.f, false, 0, 20.f, 1.f, XOR( L"hp" ) );
			auto_sniper.baim_hp.AddShowCallback( callbacks::IsAuto );
			auto_sniper.baim_hp.AddShowCallback( callbacks::IsBaimHPOn );
			RegisterElement( &auto_sniper.baim_hp, 1 );

			auto_sniper.auto_stop.setup( XOR( "auto stop" ), XOR( "auto_stop_auto_sniper" ), { XOR( "between shots" ), XOR( "in air" ) } );
			auto_sniper.auto_stop.AddShowCallback( callbacks::IsAuto );
			RegisterElement( &auto_sniper.auto_stop, 1 );
		}

		// awp
		{
			awp.hitchance_amount.setup( "hitchance", XOR( "hitchance_amount_awp" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			awp.hitchance_amount.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.hitchance_amount );

			awp.accuracy_boost.setup( "accuracy boost", XOR( "accuracy_boost_awp" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			awp.accuracy_boost.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.accuracy_boost );

			awp.hitbox.setup( XOR( "hitbox" ), XOR( "hitbox_awp" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			awp.hitbox.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.hitbox );

			awp.hitbox_air.setup( XOR( "hitbox in air" ), XOR( "hitbox_air_awp" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			awp.hitbox_air.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.hitbox_air );

			awp.air_scale.setup( XOR( "in air scale multiplier" ), XOR( "hitbox_air_scale_awp" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			awp.air_scale.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.air_scale );

			awp.scale.setup( XOR( "head scale" ), XOR( "hitbox_scale_awp" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			awp.scale.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.scale );

			awp.body_scale.setup( XOR( "body scale" ), XOR( "body_hitbox_scale_awp" ), 1.f, 100.f, true, 0, 50.f, 1.f, XOR( L"%" ) );
			awp.body_scale.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.body_scale );

			awp.minimal_damage.setup( XOR( "minimal damage" ), XOR( "minimal_damage_awp" ), 1.f, 110.f, true, 0, 40.f, 1.f, XOR( L"hp" ) );
			awp.minimal_damage.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.minimal_damage );

			awp.penetrate.setup( XOR( "penetrate walls" ), XOR( "penetrate_awp" ) );
			awp.penetrate.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.penetrate );

			awp.minimal_damage_hp.setup( XOR( "scale damage on hp" ), XOR( "minimal_damage_hp_awp" ) );
			awp.minimal_damage_hp.AddShowCallback(callbacks::IsBelowHunawp);
			awp.minimal_damage_hp.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.minimal_damage_hp );

			awp.prefer_safety.setup( XOR( "prefer safety" ), XOR( "prefer_safety_awp" ) );
			awp.prefer_safety.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.prefer_safety, 1 );

			awp.force_safety.setup( XOR( "force safety" ), XOR( "force_safety_awp" ), { XOR( "walking" ), XOR( "standing" ), XOR( "in air" ), XOR( "update" ), XOR( "potentially lethal body aim" ), XOR( "predicted landing" ) } );
			awp.force_safety.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.force_safety, 1 );

			awp.baim1.setup( XOR( "prefer body aim" ), XOR( "baim1_awp" ), { XOR( "always " ), XOR( "in air" ), XOR( "lethal x2" ) } );
			awp.baim1.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.baim1, 1 );

			awp.baim2.setup( XOR( "only body aim" ), XOR( "baim2_awp" ), { XOR( "always" ), XOR( "health" ), XOR( "in air" ) } );
			awp.baim2.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.baim2, 1 );

			awp.baim_hp.setup( "", XOR( "baim_hp_awp" ), 1.f, 50.f, false, 0, 20.f, 1.f, XOR( L"hp" ) );
			awp.baim_hp.AddShowCallback( callbacks::IsAwp );
			awp.baim_hp.AddShowCallback( callbacks::IsBaimHPOn );
			RegisterElement( &awp.baim_hp, 1 );

			awp.auto_stop.setup( XOR( "auto stop" ), XOR( "auto_stop_awp" ), { XOR( "between shots" ), XOR( "in air" ) } );
			awp.auto_stop.AddShowCallback( callbacks::IsAwp );
			RegisterElement( &awp.auto_stop, 1 );
		}

		// scout
		{
			scout.hitchance_amount.setup( "hitchance", XOR( "hitchance_scout" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			scout.hitchance_amount.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.hitchance_amount );

			scout.accuracy_boost.setup( "accuracy boost", XOR( "accuracy_boost_scout" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			scout.accuracy_boost.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.accuracy_boost );

			scout.hitbox.setup( XOR( "hitbox" ), XOR( "hitbox_scout" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			scout.hitbox.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.hitbox );

			scout.hitbox_air.setup( XOR( "hitbox in air" ), XOR( "hitbox_air_scout" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			scout.hitbox_air.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.hitbox_air );

			scout.air_scale.setup( XOR( "in air scale multiplier" ), XOR( "hitbox_air_scale_scout" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			scout.air_scale.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.air_scale );

			scout.scale.setup( XOR( "head scale" ), XOR( "hitbox_scale_scout" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			scout.scale.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.scale );

			scout.body_scale.setup( XOR( "body scale" ), XOR( "body_hitbox_scale_scout" ), 1.f, 100.f, true, 0, 50.f, 1.f, XOR( L"%" ) );
			scout.body_scale.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.body_scale );

			scout.minimal_damage.setup( XOR( "minimal damage" ), XOR( "minimal_damage_scout" ), 1.f, 110.f, true, 0, 40.f, 1.f, XOR( L"hp" ) );
			scout.minimal_damage.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.minimal_damage );

			scout.penetrate.setup( XOR( "penetrate walls" ), XOR( "penetrate_scout" ) );
			scout.penetrate.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.penetrate );

			scout.minimal_damage_hp.setup( XOR( "scale damage on hp" ), XOR( "minimal_damage_hp_scout" ) );
			scout.minimal_damage_hp.AddShowCallback(callbacks::IsBelowHun);
			scout.minimal_damage_hp.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.minimal_damage_hp );

			scout.prefer_safety.setup( XOR( "prefer safety" ), XOR( "prefer_safety_scout" ) );
			scout.prefer_safety.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.prefer_safety, 1 );

			scout.force_safety.setup( XOR( "force safety" ), XOR( "force_safety_scout" ), { XOR( "walking" ), XOR( "standing" ), XOR( "in air" ), XOR( "update" ), XOR( "potentially lethal body aim" ), XOR( "predicted landing" ) } );
			scout.force_safety.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.force_safety, 1 );

			scout.baim1.setup( XOR( "prefer body aim" ), XOR( "baim1_scout" ), { XOR( "always " ), XOR( "in air" ), XOR( "lethal x2" ) } );
			scout.baim1.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.baim1, 1 );

			scout.baim2.setup( XOR( "only body aim" ), XOR( "baim2_scout" ), { XOR( "always" ), XOR( "health" ), XOR( "in air" ) } );
			scout.baim2.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.baim2, 1 );

			scout.baim_hp.setup( "", XOR( "baim_hp_scout" ), 1.f, 50.f, false, 0, 20.f, 1.f, XOR( L"hp" ) );
			scout.baim_hp.AddShowCallback( callbacks::IsScout );
			scout.baim_hp.AddShowCallback( callbacks::IsBaimHPOn );
			RegisterElement( &scout.baim_hp, 1 );

			scout.auto_stop.setup( XOR( "auto stop" ), XOR( "auto_stop_scout" ), { XOR( "between shots" ), XOR( "in air" ) } );
			scout.auto_stop.AddShowCallback( callbacks::IsScout );
			RegisterElement( &scout.auto_stop, 1 );
		}

		// heavy pistol
		{
			heavy_pistol.hitchance_amount.setup( "hitchance", XOR( "hitchance_amount_heavy_pistol" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			heavy_pistol.hitchance_amount.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.hitchance_amount );

			heavy_pistol.accuracy_boost.setup( "accuracy boost", XOR( "accuracy_boost_heavy_pistol" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			heavy_pistol.accuracy_boost.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.accuracy_boost );

			heavy_pistol.hitbox.setup( XOR( "hitbox" ), XOR( "hitbox_heavy_pistol" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			heavy_pistol.hitbox.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.hitbox );

			heavy_pistol.hitbox_air.setup( XOR( "hitbox in air" ), XOR( "hitbox_air_heavy_pistol" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			heavy_pistol.hitbox_air.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.hitbox_air );

			heavy_pistol.air_scale.setup( XOR( "in air scale multiplier" ), XOR( "hitbox_air_scale_heavy_pistol" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			heavy_pistol.air_scale.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.air_scale );

			heavy_pistol.scale.setup( XOR( "head scale" ), XOR( "hitbox_scale_heavy_pistol" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			heavy_pistol.scale.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.scale );

			heavy_pistol.body_scale.setup( XOR( "body scale" ), XOR( "body_hitbox_scale_heavy_pistol" ), 1.f, 100.f, true, 0, 50.f, 1.f, XOR( L"%" ) );
			heavy_pistol.body_scale.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.body_scale );

			heavy_pistol.minimal_damage.setup( XOR( "minimal damage" ), XOR( "minimal_damage_heavy_pistol" ), 1.f, 110.f, true, 0, 40.f, 1.f, XOR( L"hp" ) );
			heavy_pistol.minimal_damage.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.minimal_damage );

			heavy_pistol.penetrate.setup( XOR( "penetrate walls" ), XOR( "penetrate_heavy_pistol" ) );
			heavy_pistol.penetrate.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.penetrate );

			heavy_pistol.minimal_damage_hp.setup( XOR( "scale damage on hp" ), XOR( "minimal_damage_hp_heavy_pistol" ) );
			heavy_pistol.minimal_damage_hp.AddShowCallback(callbacks::IsBelowHungenheavypistol);
			heavy_pistol.minimal_damage_hp.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.minimal_damage_hp );

			heavy_pistol.prefer_safety.setup( XOR( "prefer safety" ), XOR( "prefer_safety_heavy_pistol" ) );
			heavy_pistol.prefer_safety.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.prefer_safety, 1 );

			heavy_pistol.force_safety.setup( XOR( "force safety" ), XOR( "force_safety_heavy_pistol" ), { XOR( "walking" ), XOR( "standing" ), XOR( "in air" ), XOR( "update" ), XOR( "potentially lethal body aim" ), XOR( "predicted landing" ) } );
			heavy_pistol.force_safety.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.force_safety, 1 );

			heavy_pistol.baim1.setup( XOR( "prefer body aim" ), XOR( "baim1_heavy_pistol" ), { XOR( "always " ), XOR( "in air" ), XOR( "lethal x2" ) } );
			heavy_pistol.baim1.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.baim1, 1 );

			heavy_pistol.baim2.setup( XOR( "only body aim" ), XOR( "baim2_heavy_pistol" ), { XOR( "always" ), XOR( "health" ), XOR( "in air" ) } );
			heavy_pistol.baim2.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.baim2, 1 );

			heavy_pistol.baim_hp.setup( "", XOR( "baim_hp_heavy_pistol" ), 1.f, 50.f, false, 0, 20.f, 1.f, XOR( L"hp" ) );
			heavy_pistol.baim_hp.AddShowCallback( callbacks::IsHPistol );
			heavy_pistol.baim_hp.AddShowCallback( callbacks::IsBaimHPOn );
			RegisterElement( &heavy_pistol.baim_hp, 1 );

			heavy_pistol.auto_stop.setup( XOR( "auto stop" ), XOR( "auto_stop_heavy_pistol" ), { XOR( "between shots" ), XOR( "in air" ) } );
			heavy_pistol.auto_stop.AddShowCallback( callbacks::IsHPistol );
			RegisterElement( &heavy_pistol.auto_stop, 1 );
		}

		// pistol
		{
			pistol.hitchance_amount.setup( "hitchance", XOR( "hitchance_amount_pistol" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			pistol.hitchance_amount.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.hitchance_amount );

			pistol.accuracy_boost.setup( "accuracy boost", XOR( "accuracy_boost_pistol" ), 0.f, 100.f, true, 0, 60.f, 1.f );
			pistol.accuracy_boost.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.accuracy_boost );

			pistol.hitbox.setup( XOR( "hitbox" ), XOR( "hitbox_pistol" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			pistol.hitbox.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.hitbox );

			pistol.hitbox_air.setup( XOR( "hitbox in air" ), XOR( "hitbox_air_pistol" ), { XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ) } );
			pistol.hitbox_air.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.hitbox_air );

			pistol.air_scale.setup( XOR( "in air scale" ), XOR( "hitbox_air_scale_pistol" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			pistol.air_scale.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.air_scale );

			pistol.scale.setup( XOR( "head scale" ), XOR( "hitbox_scale_pistol" ), 1.f, 100.f, true, 0, 90.f, 1.f, XOR( L"%" ) );
			pistol.scale.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.scale );

			pistol.body_scale.setup( XOR( "body scale" ), XOR( "body_hitbox_scale_pistol" ), 1.f, 100.f, true, 0, 50.f, 1.f, XOR( L"%" ) );
			pistol.body_scale.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.body_scale );

			pistol.minimal_damage.setup( XOR( "minimal damage" ), XOR( "minimal_damage_pistol" ), 1.f, 110.f, true, 0, 40.f, 1.f, XOR( L"hp" ) );
			pistol.minimal_damage.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.minimal_damage );

			pistol.penetrate.setup( XOR( "penetrate walls" ), XOR( "penetrate_pistol" ) );
			pistol.penetrate.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.penetrate );

			pistol.minimal_damage_hp.setup( XOR( "scale damage on hp" ), XOR( "minimal_damage_hp_pistol" ) );
			pistol.minimal_damage_hp.AddShowCallback(callbacks::IsBelowHungenpistol);
			pistol.minimal_damage_hp.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.minimal_damage_hp );

			pistol.prefer_safety.setup( XOR( "prefer safety" ), XOR( "prefer_safety_pistol" ) );
			pistol.prefer_safety.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.prefer_safety, 1 );

			pistol.force_safety.setup( XOR( "force safety" ), XOR( "force_safety_pistol" ), { XOR( "walking" ), XOR( "standing" ), XOR( "in air" ), XOR( "update" ), XOR( "potentially lethal body aim" ), XOR( "predicted landing" ) } );
			pistol.force_safety.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.force_safety, 1 );

			pistol.baim1.setup( XOR( "prefer body aim" ), XOR( "baim1_pistol" ), { XOR( "always " ), XOR( "in air" ), XOR( "lethal x2" ) } );
			pistol.baim1.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.baim1, 1 );

			pistol.baim2.setup( XOR( "only body aim" ), XOR( "baim2_pistol" ), { XOR( "always" ), XOR( "health" ), XOR( "in air" ) } );
			pistol.baim2.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.baim2, 1 );

			pistol.baim_hp.setup( "", XOR( "baim_hp_pistol" ), 1.f, 50.f, false, 0, 20.f, 1.f, XOR( L"hp" ) );
			pistol.baim_hp.AddShowCallback( callbacks::IsPistol );
			pistol.baim_hp.AddShowCallback( callbacks::IsBaimHPOn );
			RegisterElement( &pistol.baim_hp, 1 );

			pistol.auto_stop.setup( XOR( "auto stop" ), XOR( "auto_stop_pistol" ), { XOR( "between shots" ), XOR( "in air" ) } );
			pistol.auto_stop.AddShowCallback( callbacks::IsPistol );
			RegisterElement( &pistol.auto_stop, 1 );
		}

		scope.setup( XOR( "auto scope" ), XOR( "scope" ), { XOR( "off" ), XOR( "always" ), XOR( "hitchance fail" ) } );
		RegisterElement( &scope, 1 );

		baim_key.setup( XOR( "force body aim" ), XOR( "baim_key" ) );
		baim_key.SetToggleCallback( callbacks::ToggleBAim );
		RegisterElement( &baim_key, 1 );

		dmg_key.setup( XOR( "damage override" ), XOR( "dmg_key" ) );
		dmg_key.SetToggleCallback( callbacks::ToggleDamage );
		RegisterElement( &dmg_key, 1 );

		dmg_ov.setup( "", XOR( "dmg_ov" ), 1.f, 100.f, false, 0, 30.f, 1.f );
		RegisterElement( &dmg_ov, 1 );

		ov_correction.setup( XOR( "override correction" ), XOR( "ov_correction" ) );
		RegisterElement( &ov_correction, 1 );

		ov_correction_col.setup(XOR("aim matrix color"), XOR("ov_correction_col"), colors::fam_green);
		RegisterElement(&ov_correction_col, 1);

		increase_firerate.setup( XOR( "increase firerate" ), XOR( "increase_firerate" ) );
		increase_firerate.SetToggleCallback( callbacks::ToggleIncreaseFirerate );
		RegisterElement( &increase_firerate, 1 );

		firerate.setup( XOR( "ticks to shift" ), XOR( "firerate" ), 7, 14, true, 0, 14, 1, XOR( L"t" ) );
		RegisterElement( &firerate, 1 );

		debug_aim_points.setup( XOR( "debug aim points" ), XOR( "debug_aim_points" ) );
		RegisterElement( &debug_aim_points, 1 );

		aim_matrix.setup( XOR( "render aim matrix" ), XOR( "aim_matrix" ) );
		RegisterElement( &aim_matrix, 1 );

		aim_matrix_col.setup( XOR( "aim matrix color" ), XOR( "aim_matrix_col" ), colors::white );
		RegisterElement( &aim_matrix_col, 1 );
	}
};

class AntiAimTab : public Tab
{
public:
	// col 1.
	Checkbox enable;
	MultiDropdown edge;
	Dropdown mode;

	Dropdown pitch_stand;
	Dropdown yaw_stand;
	Slider   jitter_range_stand;
	Slider   rot_range_stand;
	Slider   rot_speed_stand;
	Slider   rand_update_stand;
	Dropdown dir_stand;
	Slider   dir_time_stand;
	Slider   dir_custom_stand;
	Dropdown base_angle_stand;
	Dropdown body_fake_stand;
	Checkbox disable_body_fake_duck;
	Keybind  desync_key;

	Dropdown pitch_walk;
	Dropdown yaw_walk;
	Slider   jitter_range_walk;
	Slider   rot_range_walk;
	Slider   rot_speed_walk;
	Slider   rand_update_walk;
	Dropdown dir_walk;
	Slider	 dir_time_walk;
	Slider   dir_custom_walk;
	Dropdown base_angle_walk;

	Dropdown pitch_air;
	Dropdown yaw_air;
	Slider   jitter_range_air;
	Slider   rot_range_air;
	Slider   rot_speed_air;
	Slider   rand_update_air;
	Dropdown dir_air;
	Slider   dir_time_air;
	Slider   dir_custom_air;
	Dropdown base_angle_air;

	// col 2.
	Dropdown fake_yaw;
	Slider	 fake_relative;
	Slider	 fake_jitter_range;
	Dropdown fake_desync;
	Keybind  desync_inverter;
	Slider	 desync_yaw_add;

	Dropdown avoid_lby;
	Checkbox no_avoid_lby_in_air;

	Checkbox      lag_enable;
	MultiDropdown lag_active;
	Dropdown      lag_mode;
	Slider        lag_limit;
	Slider        lag_airdelay;
	Checkbox      lag_land;

	Keybind  manual_front;
	Keybind  manual_back;
	Keybind  manual_right;
	Keybind  manual_left;
	Colorpicker manual_color;

public:
	void init( )
	{
		SetTitle( XOR( "anti-aim" ) );

		enable.setup( XOR( "enable" ), XOR( "enable" ) );
		RegisterElement( &enable );

		edge.setup( XOR( "edge" ), XOR( "edge_yaw" ), { XOR( "stand" ), XOR( "walk" ), XOR( "air" ) } );
		RegisterElement( &edge );

		mode.setup( "", XOR( "mode" ), { XOR( "stand" ), XOR( "walk" ), XOR( "air" ) }, false );
		RegisterElement( &mode );

		// stand.
		pitch_stand.setup( XOR( "pitch" ), XOR( "pitch_stnd" ), { XOR( "off" ), XOR( "down" ), XOR( "up" ), XOR( "random" ), XOR( "ideal" ) } );
		pitch_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		RegisterElement( &pitch_stand );

		yaw_stand.setup(XOR("yaw"), XOR("yaw_stnd"), { XOR("off"), XOR("direction"), XOR("jitter"), XOR("center jitter"), XOR("rotate"), XOR("random") });
		yaw_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		RegisterElement( &yaw_stand );

		jitter_range_stand.setup( "", XOR( "jitter_range_stnd" ), 1.f, 180.f, false, 0, 45.f, 5.f, XOR( L"°" ) );
		jitter_range_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		jitter_range_stand.AddShowCallback( callbacks::IsStandYawJitter );
		RegisterElement( &jitter_range_stand );

		rot_range_stand.setup( "", XOR( "rot_range_stnd" ), 0.f, 360.f, false, 0, 360.f, 5.f, XOR( L"°" ) );
		rot_range_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		rot_range_stand.AddShowCallback( callbacks::IsStandYawRotate );
		RegisterElement( &rot_range_stand );

		rot_speed_stand.setup( "", XOR( "rot_speed_stnd" ), 1.f, 100.f, false, 0, 10.f, 1.f, XOR( L"%" ) );
		rot_speed_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		rot_speed_stand.AddShowCallback( callbacks::IsStandYawRotate );
		RegisterElement( &rot_speed_stand );

		rand_update_stand.setup( "", XOR( "rand_update_stnd" ), 0.f, 1.f, false, 1, 0.f, 0.1f );
		rand_update_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		rand_update_stand.AddShowCallback( callbacks::IsStandYawRnadom );
		RegisterElement( &rand_update_stand );

		dir_stand.setup( XOR( "direction" ), XOR( "dir_stnd" ), { XOR( "auto" ), XOR( "backwards" ), XOR( "left" ), XOR( "right" ), XOR( "custom" ) } );
		dir_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		dir_stand.AddShowCallback( callbacks::HasStandYaw );
		RegisterElement( &dir_stand );

		dir_time_stand.setup( "", XOR( "dir_time_stnd" ), 0.f, 10.f, false, 0, 0.f, 1.f, XOR( L"s" ) );
		dir_time_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		dir_time_stand.AddShowCallback( callbacks::HasStandYaw );
		dir_time_stand.AddShowCallback( callbacks::IsStandDirAuto );
		RegisterElement( &dir_time_stand );

		dir_custom_stand.setup( "", XOR( "dir_custom_stnd" ), -180.f, 180.f, false, 0, 0.f, 5.f, XOR( L"°" ) );
		dir_custom_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		dir_custom_stand.AddShowCallback( callbacks::HasStandYaw );
		dir_custom_stand.AddShowCallback( callbacks::IsStandDirCustom );
		RegisterElement( &dir_custom_stand );

		base_angle_stand.setup( XOR( "at targets" ), XOR( "base_angle_stand" ), { XOR( "off" ), XOR( "static" ), XOR( "away crosshair" ), XOR( "away distance" ) } );
		base_angle_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		base_angle_stand.AddShowCallback( callbacks::HasStandYaw );
		RegisterElement( &base_angle_stand );

		// walk.
		pitch_walk.setup( XOR( "pitch" ), XOR( "pitch_walk" ), { XOR( "off" ), XOR( "down" ), XOR( "up" ), XOR( "random" ), XOR( "ideal" ) } );
		pitch_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		RegisterElement( &pitch_walk );

		yaw_walk.setup(XOR("yaw"), XOR("yaw_walk"), { XOR("off"), XOR("direction"), XOR("jitter"), XOR("center jitter"), XOR("rotate"), XOR("random") });
		yaw_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		RegisterElement( &yaw_walk );

		jitter_range_walk.setup( "", XOR( "jitter_range_walk" ), 1.f, 180.f, false, 0, 45.f, 5.f, XOR( L"°" ) );
		jitter_range_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		jitter_range_walk.AddShowCallback( callbacks::IsWalkYawJitter );
		RegisterElement( &jitter_range_walk );

		rot_range_walk.setup( "", XOR( "rot_range_walk" ), 0.f, 360.f, false, 0, 360.f, 5.f, XOR( L"°" ) );
		rot_range_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		rot_range_walk.AddShowCallback( callbacks::IsWalkYawRotate );
		RegisterElement( &rot_range_walk );

		rot_speed_walk.setup( "", XOR( "rot_speed_walk" ), 1.f, 100.f, false, 0, 10.f, 1.f, XOR( L"%" ) );
		rot_speed_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		rot_speed_walk.AddShowCallback( callbacks::IsWalkYawRotate );
		RegisterElement( &rot_speed_walk );

		rand_update_walk.setup( "", XOR( "rand_update_walk" ), 0.f, 1.f, false, 1, 0.f, 0.1f );
		rand_update_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		rand_update_walk.AddShowCallback( callbacks::IsWalkYawRandom );
		RegisterElement( &rand_update_walk );

		dir_walk.setup( XOR( "direction" ), XOR( "dir_walk" ), { XOR( "auto" ), XOR( "backwards" ), XOR( "left" ), XOR( "right" ), XOR( "custom" ) } );
		dir_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		dir_walk.AddShowCallback( callbacks::WalkHasYaw );
		RegisterElement( &dir_walk );

		dir_time_walk.setup( "", XOR( "dir_time_walk" ), 0.f, 10.f, false, 0, 0.f, 1.f, XOR( L"s" ) );
		dir_time_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		dir_time_walk.AddShowCallback( callbacks::WalkHasYaw );
		dir_time_walk.AddShowCallback( callbacks::IsWalkDirAuto );
		RegisterElement( &dir_time_walk );

		dir_custom_walk.setup( "", XOR( "dir_custom_walk" ), -180.f, 180.f, false, 0, 0.f, 5.f, XOR( L"°" ) );
		dir_custom_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		dir_custom_walk.AddShowCallback( callbacks::WalkHasYaw );
		dir_custom_walk.AddShowCallback( callbacks::IsWalkDirCustom );
		RegisterElement( &dir_custom_walk );

		base_angle_walk.setup( XOR( "at targets" ), XOR( "base_angle_walk" ), { XOR( "off" ), XOR( "static" ), XOR( "away crosshair" ), XOR( "away distance" ) } );
		base_angle_walk.AddShowCallback( callbacks::IsAntiAimModeWalk );
		base_angle_walk.AddShowCallback( callbacks::WalkHasYaw );
		RegisterElement( &base_angle_walk );

		// air.
		pitch_air.setup( XOR( "pitch" ), XOR( "pitch_air" ), { XOR( "off" ), XOR( "down" ), XOR( "up" ), XOR( "random" ), XOR( "ideal" ) } );
		pitch_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		RegisterElement( &pitch_air );

		yaw_air.setup(XOR("yaw"), XOR("yaw_air"), { XOR("off"), XOR("direction"), XOR("jitter"), XOR("center jitter"), XOR("rotate"), XOR("random") });
		yaw_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		RegisterElement( &yaw_air );

		jitter_range_air.setup( "", XOR( "jitter_range_air" ), 1.f, 180.f, false, 0, 45.f, 5.f, XOR( L"°" ) );
		jitter_range_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		jitter_range_air.AddShowCallback( callbacks::IsAirYawJitter );
		RegisterElement( &jitter_range_air );

		rot_range_air.setup( "", XOR( "rot_range_air" ), 0.f, 360.f, false, 0, 360.f, 5.f, XOR( L"°" ) );
		rot_range_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		rot_range_air.AddShowCallback( callbacks::IsAirYawRotate );
		RegisterElement( &rot_range_air );

		rot_speed_air.setup( "", XOR( "rot_speed_air" ), 1.f, 100.f, false, 0, 10.f, 1.f, XOR( L"%" ) );
		rot_speed_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		rot_speed_air.AddShowCallback( callbacks::IsAirYawRotate );
		RegisterElement( &rot_speed_air );

		rand_update_air.setup( "", XOR( "rand_update_air" ), 0.f, 1.f, false, 1, 0.f, 0.1f );
		rand_update_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		rand_update_air.AddShowCallback( callbacks::IsAirYawRandom );
		RegisterElement( &rand_update_air );

		dir_air.setup( XOR( "direction" ), XOR( "dir_air" ), { XOR( "auto" ), XOR( "backwards" ), XOR( "left" ), XOR( "right" ), XOR( "custom" ) } );
		dir_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		dir_air.AddShowCallback( callbacks::AirHasYaw );
		RegisterElement( &dir_air );

		dir_time_air.setup( "", XOR( "dir_time_air" ), 0.f, 10.f, false, 0, 0.f, 1.f, XOR( L"s" ) );
		dir_time_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		dir_time_air.AddShowCallback( callbacks::AirHasYaw );
		dir_time_air.AddShowCallback( callbacks::IsAirDirAuto );
		RegisterElement( &dir_time_air );

		dir_custom_air.setup( "", XOR( "dir_custom_air" ), -180.f, 180.f, false, 0, 0.f, 5.f, XOR( L"°" ) );
		dir_custom_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		dir_custom_air.AddShowCallback( callbacks::AirHasYaw );
		dir_custom_air.AddShowCallback( callbacks::IsAirDirCustom );
		RegisterElement( &dir_custom_air );

		base_angle_air.setup( XOR( "at targets" ), XOR( "base_angle_air" ), { XOR( "off" ), XOR( "static" ), XOR( "away crosshair" ), XOR( "away distance" ) } );
		base_angle_air.AddShowCallback( callbacks::IsAntiAimModeAir );
		base_angle_air.AddShowCallback( callbacks::AirHasYaw );
		RegisterElement( &base_angle_air );

		// col2.
		body_fake_stand.setup( XOR( "fake body" ), XOR( "body_fake_stnd" ), { XOR( "off" ), XOR( "110" ), XOR( "180" ), XOR( "twist" ), XOR( "beta" ), XOR( "v" ), XOR( "direction" ) } );
		body_fake_stand.AddShowCallback( callbacks::IsAntiAimModeStand );
		body_fake_stand.AddShowCallback( callbacks::HasStandYaw );
		RegisterElement( &body_fake_stand, 1 );

		disable_body_fake_duck.setup( XOR( "disable fake body on duck" ), XOR( "disable_body_fake_duck" ) );
		disable_body_fake_duck.AddShowCallback( callbacks::IsAntiAimModeStand );
		disable_body_fake_duck.AddShowCallback( callbacks::HasStandYaw );
		RegisterElement( &disable_body_fake_duck, 1 );

		fake_yaw.setup( XOR( "fake yaw" ), XOR( "fake_yaw" ), { XOR( "off" ), XOR( "default" ), XOR( "relative" ), XOR( "jitter" ), XOR( "rotate" ), XOR( "random" ), XOR( "local view" ) } );
		RegisterElement( &fake_yaw, 1 );

		fake_relative.setup( "", XOR( "fake_relative" ), -90.f, 90.f, false, 0, 0.f, 5.f, XOR( L"°" ) );
		fake_relative.AddShowCallback( callbacks::IsFakeAntiAimRelative );
		RegisterElement( &fake_relative, 1 );

		fake_jitter_range.setup( "", XOR( "fake_jitter_range" ), 0.f, 90.f, false, 0, 0.f, 5.f, XOR( L"°" ) );
		fake_jitter_range.AddShowCallback( callbacks::IsFakeAntiAimJitter );
		RegisterElement( &fake_jitter_range, 1 );

		fake_desync.setup( XOR( "desync" ), XOR( "fake_desync" ), { XOR( "off" ), XOR( "lby" ), XOR( "micromove" ), XOR( "lby + walk" ) } );
		RegisterElement( &fake_desync, 1 );

		desync_yaw_add.setup( "desync yaw add", XOR( "desync_yaw_add" ), 0.f, 90.f, false, 0, 0.f, 5.f, XOR( L"°" ) );
		RegisterElement( &desync_yaw_add, 1 );

		desync_key.setup( XOR( "desyncwalk" ), XOR( "desync_key" ) );
		desync_key.SetToggleCallback( callbacks::UpdateExploit );
		RegisterElement( &desync_key, 1 );

		desync_inverter.setup( XOR( "inverter" ), XOR( "desync_inverter" ) );
		desync_inverter.SetToggleCallback( callbacks::UpdateInverter );
		RegisterElement( &desync_inverter, 1 );

		avoid_lby.setup( XOR( "avoid lby" ), XOR( "avoid_lby" ), { XOR( "off" ), XOR( "normal" ), XOR( "avoid land" ) } );
		RegisterElement( &avoid_lby, 1 );

		no_avoid_lby_in_air.setup( XOR( "disable avoid lby in air" ), XOR( "no_avoid_lby_in_air" ) );
		RegisterElement( &no_avoid_lby_in_air, 1 );

		// col 2.
		lag_enable.setup( XOR( "fake-lag" ), XOR( "lag_enable" ) );
		RegisterElement( &lag_enable, 1 );

		lag_active.setup( "", XOR( "lag_active" ), { XOR( "move" ), XOR( "air" ), XOR( "stand" ), XOR( "crouch" ), XOR( "peek" ) }, false );
		RegisterElement( &lag_active, 1 );

		lag_mode.setup( "", XOR( "lag_mode" ), { XOR( "max" ), XOR( "break" ), XOR( "random" ), XOR( "break fluctuate" ) }, false );
		RegisterElement( &lag_mode, 1 );

		lag_limit.setup( XOR( "limit" ), XOR( "lag_limit" ), 2, 16, true, 0, 2, 1.f );
		RegisterElement( &lag_limit, 1 );

		lag_airdelay.setup( XOR( "air lag delay" ), XOR( "lag_airdelay" ), 0, 2000, true, 0, 0, 50.f, XOR( L"ms" ) );
		RegisterElement( &lag_airdelay, 1 );

		lag_land.setup( XOR( "on land" ), XOR( "lag_land" ) );
		RegisterElement( &lag_land, 1 );

		manual_front.setup( XOR( "manual front" ), XOR( "manual_front" ) );
		manual_front.SetToggleCallback( callbacks::UpdateFront );
		RegisterElement( &manual_front );

		manual_back.setup( XOR( "manual back" ), XOR( "manual_back" ) );
		manual_back.SetToggleCallback( callbacks::UpdateBack );
		RegisterElement( &manual_back );

		manual_right.setup( XOR( "manual right" ), XOR( "manual_right" ) );
		manual_right.SetToggleCallback( callbacks::UpdateRight );
		RegisterElement( &manual_right );

		manual_left.setup( XOR( "manual left" ), XOR( "manual_left" ) );
		manual_left.SetToggleCallback( callbacks::UpdateLeft );
		RegisterElement( &manual_left );

		manual_color.setup( "manual aa color", "manual_color", colors::transparent_light_blue, nullptr );
		RegisterElement( &manual_color );
	}
};

class PlayersTab : public Tab
{
public:
	Dropdown      tab;

	Checkbox      box_enemy_enable;
	Checkbox      box_friendly_enable;
	Colorpicker   box_enemy;
	Colorpicker   box_friendly;
	Checkbox      dormant;
	Checkbox      offscreen;
	Colorpicker   offscreen_color;
	Checkbox name_enemy;
	Checkbox name_friendly;
	Colorpicker   name_color_enemy;
	Colorpicker   name_color_friendly;
	MultiDropdown health_enemy;
	MultiDropdown health_friendly;
	Colorpicker   health_enemy_color;
	Colorpicker   health_friendly_color;
	MultiDropdown flags_enemy;
	MultiDropdown flags_friendly;
	Checkbox      weapon_enemy;
	Checkbox      weapon_friendly;
	Dropdown      weapon_mode_enemy;
	Dropdown      weapon_mode_friendly;
	Checkbox      ammo;
	Colorpicker   ammo_color;
	Checkbox      lby_update;
	Colorpicker   lby_update_color;

	// col2.
	MultiDropdown skeleton_enemy_enable;
	Checkbox      skeleton_friendly_enable;
	Colorpicker   skeleton_enemy;
	Colorpicker   skeleton_friendly;

	MultiDropdown glow_enemy_enable;
	MultiDropdown glow_friendly_enable;

	Colorpicker   glow_enemy;
	Colorpicker   glow_friendly;
	MultiDropdown chams_enemy;
	Colorpicker   chams_enemy_vis;
	Colorpicker   chams_enemy_invis;
	Checkbox      chams_enemy_history;
	Colorpicker   chams_enemy_history_col;

	MultiDropdown chams_friendly;
	Colorpicker   chams_friendly_vis;
	Colorpicker   chams_friendly_invis;
	MultiDropdown chams_fake;
	Colorpicker   chams_fake_col;
	MultiDropdown chams_local;
	Colorpicker   chams_local_col;
	Checkbox      chams_local_scope;

public:
	void init( )
	{
		SetTitle( XOR( "players" ) );

		tab.setup( XOR( "players" ), XOR( "tab" ), { XOR( "enemy" ), XOR( "friendly" ), XOR( "local" ) } );
		RegisterElement( &tab );

		dormant.setup( XOR( "render while dormant" ), XOR( "dormant" ) );
		dormant.AddShowCallback( callbacks::IsNotLocal );
		RegisterElement( &dormant );

		name_enemy.setup( XOR( "name" ), XOR( "name_enemy" ) );
		name_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &name_enemy );

		name_color_enemy.setup( XOR( "color" ), XOR( "name_color_enemy" ), colors::white );
		name_color_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &name_color_enemy );



		name_friendly.setup( XOR( "name" ), XOR( "name_friendly" ) );
		name_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &name_friendly );



		name_color_friendly.setup( XOR( "color" ), XOR( "name_color_friendly" ), colors::white );
		name_color_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &name_color_friendly );

		box_enemy_enable.setup( XOR( "bounding box" ), XOR( "box_enemy_enable" ) );
		box_enemy_enable.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &box_enemy_enable );

		box_enemy.setup( XOR( "box color" ), XOR( "box_enemy" ), { colors::white } );
		box_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &box_enemy );

		offscreen.setup( XOR( "offscreen esp" ), XOR( "offscreen" ) );
		offscreen.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &offscreen );

		offscreen_color.setup( XOR( "offscreen esp color" ), XOR( "offscreen_color" ), colors::white, nullptr );
		offscreen_color.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &offscreen_color );

		box_friendly_enable.setup( XOR( "bounding box" ), XOR( "box_friendly_enable" ) );
		box_friendly_enable.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &box_friendly_enable );

		box_friendly.setup( XOR( "box color" ), XOR( "box_friendly" ), { colors::white } );
		box_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &box_friendly );

		health_enemy.setup( XOR( "health" ), XOR( "health_enemy" ), { XOR( "enable" ), XOR( "custom" ) } );
		health_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &health_enemy );

		health_enemy_color.setup( XOR( "color" ), XOR( "health_enemy_color" ), { 255, 255, 255 } );
		health_enemy_color.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &health_enemy_color );

		health_friendly.setup( XOR( "health" ), XOR( "health_friendly" ), { XOR( "enable" ), XOR( "custom" ) } );
		health_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &health_friendly );

		health_friendly_color.setup( XOR( "color" ), XOR( "health_friendly_color" ), { 255, 255, 255 } );
		health_friendly_color.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &health_friendly_color );

		flags_enemy.setup( XOR( "flags enemy" ), XOR( "flags_enemy" ), { XOR( "money" ), XOR( "armor" ), XOR( "scoped" ), XOR( "flashed" ), XOR( "reloading" ), XOR( "bomb" ), XOR( "lethal" ), XOR( "rax" ), XOR( "family" ), XOR( "moneybot" ), XOR( "resolve confidence" ) } );
		flags_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &flags_enemy );

		flags_friendly.setup( XOR( "flags friendly" ), XOR( "flags_friendly" ), { XOR( "money" ), XOR( "armor" ), XOR( "scoped" ), XOR( "flashed" ), XOR( "reloading" ), XOR( "bomb" ) } );
		flags_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &flags_friendly );

		weapon_enemy.setup( XOR( "weapon" ), XOR( "weapon_enemy" ) );
		weapon_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &weapon_enemy );

		weapon_mode_enemy.setup( "", XOR( "weapon_mode_enemy" ), { XOR( "text" ), XOR( "icon" ) }, false );
		weapon_mode_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &weapon_mode_enemy );

		weapon_friendly.setup( XOR( "weapon" ), XOR( "weapon_friendly" ) );
		weapon_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &weapon_friendly );

		weapon_mode_friendly.setup( "", XOR( "weapon_mode_friendly" ), { XOR( "text" ), XOR( "icon" ) }, false );
		weapon_mode_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &weapon_mode_friendly );

		ammo.setup( XOR( "ammo" ), XOR( "ammo" ) );
		ammo.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &ammo, 1 );

		ammo_color.setup( XOR( "color" ), XOR( "ammo_color" ), { 60, 120, 180 } );
		ammo_color.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &ammo_color, 1 );

		lby_update.setup( XOR( "lby update" ), XOR( "lby_update" ) );
		lby_update.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &lby_update, 1 );

		lby_update_color.setup( XOR( "color" ), XOR( "lby_update_color" ), colors::orange );
		lby_update_color.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &lby_update_color, 1 );

		// col2.
		skeleton_enemy_enable.setup( XOR( "skeleton" ), XOR( "skeleton_enemy_enable" ), { XOR( "normal" ), XOR( "history" ) } );
		skeleton_enemy_enable.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &skeleton_enemy_enable, 1 );

		skeleton_enemy.setup( XOR( "skeleton enemy color" ), XOR( "skeleton_enemy" ), { 255, 255, 255, 255 }, nullptr );
		skeleton_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &skeleton_enemy, 1 );

		skeleton_friendly_enable.setup( XOR( "skeleton" ), XOR( "skeleton_friendly_enable" ) );
		skeleton_friendly_enable.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &skeleton_friendly_enable, 1 );

		skeleton_friendly.setup( XOR( "color" ), XOR( "skeleton_friendly" ), { 255, 255, 255 } );
		skeleton_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &skeleton_friendly, 1 );

		glow_enemy_enable.setup( XOR( "glow" ), XOR( "glow" ), { XOR( "normal" ), XOR( "stencil" ), XOR( "overlay" ) } );
		glow_enemy_enable.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &glow_enemy_enable, 1 );

		glow_enemy.setup( XOR( "glow enemy color" ), XOR( "glow_enemy" ), { 180, 60, 120 }, nullptr );
		glow_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &glow_enemy, 1 );

		glow_friendly_enable.setup( XOR( "glow" ), XOR( "glow_friendly_enable" ), { XOR( "normal" ), XOR( "stencil" ), XOR( "overlay" ) } );
		glow_friendly_enable.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &glow_friendly_enable, 1 );

		glow_friendly.setup( XOR( "glow friend color" ), XOR( "glow_friendly" ), { 180, 60, 120 }, nullptr );
		glow_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &glow_friendly, 1 );

		chams_enemy.setup( XOR( "chams enemy" ), XOR( "chams_enemy" ), { XOR( "visible" ), XOR( "invisible" ), XOR( "flat" ), XOR( "metallic" ) } );
		chams_enemy.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &chams_enemy, 1 );

		chams_enemy_vis.setup( XOR( "chams enemy color visible" ), XOR( "chams_enemy_vis" ), { 150, 200, 60 }, nullptr );
		chams_enemy_vis.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &chams_enemy_vis, 1 );

		chams_enemy_invis.setup( XOR( "chams enemy color invisible" ), XOR( "chams_enemy_invis" ), { 60, 120, 180 }, nullptr );
		chams_enemy_invis.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &chams_enemy_invis, 1 );

		chams_enemy_history.setup( XOR( "chams history" ), XOR( "chams_history" ) );
		chams_enemy_history.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &chams_enemy_history, 1 );

		chams_enemy_history_col.setup( XOR( "chams enemy history color" ), XOR( "chams_history_col" ), { 255, 255, 255 }, nullptr );
		chams_enemy_history_col.AddShowCallback( callbacks::IsEnemy );
		RegisterElement( &chams_enemy_history_col, 1 );

		chams_friendly.setup( XOR( "chams friendly" ), XOR( "chams_friendly" ), { XOR( "visible" ), XOR( "invisible" ), XOR( "flat" ), XOR( "metallic" ) } );
		chams_friendly.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &chams_friendly, 1 );

		chams_friendly_vis.setup( XOR( "chams friend color visible" ), XOR( "chams_friendly_vis" ), { 255, 200, 0 }, nullptr );
		chams_friendly_vis.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &chams_friendly_vis, 1 );

		chams_friendly_invis.setup( XOR( "chams friend color invisible" ), XOR( "chams_friendly_invis" ), { 255, 50, 0 }, nullptr );
		chams_friendly_invis.AddShowCallback( callbacks::IsFriendly );
		RegisterElement( &chams_friendly_invis, 1 );

		chams_local.setup( XOR( "chams local" ), XOR( "chams_local" ), { XOR( "enable" ), XOR( "flat" ), XOR( "metallic" ) } );
		chams_local.AddShowCallback( callbacks::IsLocal );
		RegisterElement( &chams_local, 1 );

		chams_local_col.setup( XOR( "chams local color" ), XOR( "chams_local_col" ), { 255, 255, 255 }, nullptr );
		chams_local_col.AddShowCallback( callbacks::IsLocal );
		RegisterElement( &chams_local_col, 1 );

		chams_local_scope.setup( XOR( "blend when scoped" ), XOR( "chams_local_scope" ) );
		chams_local_scope.AddShowCallback( callbacks::IsLocal );
		RegisterElement( &chams_local_scope, 1 );

		chams_fake.setup( XOR( "chams fake" ), XOR( "chams_fake" ), { XOR( "enable" ), XOR( "flat" ), XOR( "metallic" ) } );
		chams_fake.AddShowCallback( callbacks::IsLocal );
		RegisterElement( &chams_fake, 1 );

		chams_fake_col.setup( XOR( "chams fake color" ), XOR( "chams_fake_col" ), { 255, 255, 255 }, nullptr );
		chams_fake_col.AddShowCallback( callbacks::IsLocal );
		RegisterElement( &chams_fake_col, 1 );
	}
};

class VisualsTab : public Tab
{
public:
	Checkbox      items;
	Checkbox      ammo;
	Colorpicker   item_color;
	Checkbox      proj;
	Colorpicker   proj_safe_color;
	Colorpicker   proj_dangerous_color;
	MultiDropdown planted_c4;
	Checkbox      disableteam;
	MultiDropdown world;
	Colorpicker   world_color;
	Dropdown      sky_box;
	Colorpicker   sky_color;
	Slider        prop_transparency;
	Button        modulate_world;
	Checkbox      enemy_radar;
	//MultiDropdown local_aa_lines;
	//Colorpicker color_real;
	//Colorpicker color_lby;

	// col2.
	MultiDropdown removals;

	Checkbox      fov;
	Slider        fov_amt;
	Checkbox      fov_scoped;
	Slider        fov_scoped1;
	Checkbox      viewmodel_fov;
	Slider        viewmodel_fov_amt;
	Slider        viewmodel_x;
	Slider        viewmodel_y;
	Slider        viewmodel_z;
	Checkbox      spectators;
	Checkbox      force_xhair;
	Checkbox      spread_xhair;
	Colorpicker   spread_xhair_col;
	Checkbox      pen_crosshair;
	MultiDropdown indicators;
	Checkbox      tracers; //grenade simulation... stupid name, i know.
	Colorpicker   grenade_prediction_color; //grenade simulation color
	Checkbox      grenade_trajectory; //grenade trajectory
	Colorpicker	  grenade_trajectory_color;
	Checkbox      impact_beams;
	Colorpicker   impact_beams_color;
	Colorpicker   impact_beams_hurt_color;
	Slider        impact_beams_time;
	Keybind       thirdperson;
	Slider        thirdperson_distance;
	MultiDropdown bullet_impacts;
	Colorpicker   client_impact_color;
	Colorpicker   server_impact_color;
	Slider        impact_size;

public:
	void init( )
	{
		SetTitle( XOR( "visuals" ) );

		sky_box.setup( XOR( "skyboxes" ), XOR( "sky_box" ), { XOR( "default" ), XOR( "tibet" ), XOR( "embassy" ), XOR( "italy" ), XOR( "daylight" ), XOR( "cloudy" ), XOR( "night 1" ), XOR( "night 2" ), XOR( "night Flat" ), XOR( "day hd" ), XOR( "day" ), XOR( "rural" ), XOR( "vertigo hd" ), XOR( "vertigo blue hd" ), XOR( "vertigo" ),XOR( "vietnam" ),XOR( "dusty sky" ),XOR( "jungle" ),XOR( "nuke" ),XOR( "office" ) } );
		RegisterElement( &sky_box );

		world.setup( XOR( "world" ), XOR( "world" ), { XOR( "world modulate" ), XOR( "fullbright" ),XOR( "sky" ),XOR( "soft shadows" ), XOR("lean")});
		RegisterElement( &world );

		/*Colorpicker setup: label, file id, color, color ptr, use_label*/
		world_color.setup( XOR( "world" ), XOR( "world_color" ), { 255, 255, 255, 255 }, nullptr );
		world_color.AddShowCallback(callbacks::worldenabled);
		RegisterElement( &world_color );

		sky_color.setup( XOR( "sky" ), XOR( "sky_color" ), colors::white );
		sky_color.AddShowCallback(callbacks::skyenabled);
		RegisterElement( &sky_color );

		prop_transparency.setup( XOR( "prop transparency" ), XOR( "prop_transparency" ), 0, 75, true, 0, 0, 1.f, XOR( L"%" ) );
		RegisterElement( &prop_transparency );

		modulate_world.setup( XOR( "modulate world" ) );
		modulate_world.SetCallback( Visuals::ModulateWorld );
		RegisterElement( &modulate_world );

		items.setup( XOR( "dropped weapons" ), XOR( "items" ) );
		RegisterElement( &items );

		ammo.setup( XOR( "dropped weapons ammo" ), XOR( "ammo" ) );
		RegisterElement( &ammo );

		item_color.setup( XOR( "color" ), XOR( "item_color" ), colors::white );
		item_color.AddShowCallback(callbacks::dropped_weapon);
		RegisterElement( &item_color );

		proj.setup( XOR( "projectiles" ), XOR( "proj" ) );
		RegisterElement( &proj );

		proj_safe_color.setup(XOR("safe projectiles color"), XOR("proj_safe_color"), Color(0, 147, 255));
		proj_safe_color.AddShowCallback(callbacks::projectile1);
		RegisterElement(&proj_safe_color);


		proj_dangerous_color.setup(XOR("dangerous projectiles color"), XOR("proj_dangerous_color"), colors::white);
		proj_dangerous_color.AddShowCallback(callbacks::projectile1);
		RegisterElement(&proj_dangerous_color);



		planted_c4.setup( XOR( "planted c4" ), XOR( "planted_c4" ), { XOR( "on screen (2D)" ), XOR( "on bomb (3D)" ) } );
		RegisterElement( &planted_c4 );

		disableteam.setup( XOR( "do not render teammates" ), XOR( "disableteam" ) );
		RegisterElement( &disableteam );

		enemy_radar.setup( XOR( "force enemies on radar" ), XOR( "enemy_radar" ) );
		RegisterElement( &enemy_radar );

		bullet_impacts.setup( XOR( "bullet impacts" ), XOR( "bullet_impacts" ), { XOR( "client" ), XOR( "server" ) } );

		RegisterElement( &bullet_impacts );

		client_impact_color.setup( XOR( "client impact color" ), XOR( "client_impact_color" ), { 255, 0, 0, 125 }, nullptr );
		client_impact_color.AddShowCallback(callbacks::bullet_impactz);

		RegisterElement( &client_impact_color );

		server_impact_color.setup( XOR( "server impact color" ), XOR( "server_impact_color" ), { 0, 0, 255, 125 }, nullptr );
		server_impact_color.AddShowCallback(callbacks::bullet_impactz1);
		RegisterElement( &server_impact_color );

		impact_size.setup( "impact size", XOR( "impact_size" ), 5.f, 20.f, true, 0, 20.f, 1.f );
		impact_size.AddShowCallback(callbacks::bullet_impactz2);
		RegisterElement( &impact_size );


		removals.setup(XOR("removals"), XOR("removals"), { XOR("visual shake"), XOR("visual punch"),XOR("visual smoke"),XOR("visual fog"),XOR("visual flashbangs"),XOR("visual scope") });
		RegisterElement(&removals,1);




		fov.setup( XOR( "override fov" ), XOR( "fov" ) );
		RegisterElement( &fov, 1 );

		fov_amt.setup( "", XOR( "fov_amt" ), 60.f, 140.f, false, 0, 90.f, 1.f, XOR( L"°" ) );
		fov_amt.AddShowCallback(callbacks::fov_enabled);
		RegisterElement( &fov_amt, 1 );

		fov_scoped.setup( XOR( "override scoped fov" ), XOR( "fov_scoped" ) );
		RegisterElement( &fov_scoped, 1 );

		fov_scoped1.setup("override fov scoped", XOR("fov_scoped_amt"), 0.f, 100.f, true, 0, 100.f, 1.f, XOR(L""));
		fov_scoped1.AddShowCallback(callbacks::override_fov_enabled);
		RegisterElement(&fov_scoped1, 1);

		viewmodel_fov.setup( XOR( "override viewmodel fov" ), XOR( "viewmodel_fov" ) );
		RegisterElement( &viewmodel_fov, 1 );

		viewmodel_fov_amt.setup( "", XOR( "viewmodel_fov_amt" ), 60.f, 140.f, false, 0, 90.f, 1.f, XOR( L"°" ) );
		viewmodel_fov_amt.AddShowCallback(callbacks::viewmodel_enabled);
		RegisterElement( &viewmodel_fov_amt, 1 );

		viewmodel_x.setup( "x axis", XOR( "viewmodel_x" ), -25, 25, false, 0, 5.f, 1.f );
		viewmodel_x.AddShowCallback(callbacks::viewmodel_enabled);

		RegisterElement( &viewmodel_x, 1 );

		viewmodel_y.setup( "y axis", XOR( "viewmodel_y" ), -25, 25, false, 0, 0.f, 1.f );
		viewmodel_y.AddShowCallback(callbacks::viewmodel_enabled);

		RegisterElement( &viewmodel_y, 1 );

		viewmodel_z.setup( "z axis", XOR( "viewmodel_z" ), -25, 25, false, 0, -3.f, 1.f );
		viewmodel_z.AddShowCallback(callbacks::viewmodel_enabled);

		RegisterElement( &viewmodel_z, 1 );

		spectators.setup( XOR( "show spectator list" ), XOR( "spectators" ) );

		RegisterElement( &spectators, 1 );

		force_xhair.setup( XOR( "force crosshair" ), XOR( "force_xhair" ) );
		RegisterElement( &force_xhair, 1 );

		spread_xhair.setup( XOR( "visualize spread" ), XOR( "spread_xhair" ) );
		RegisterElement( &spread_xhair, 1 );

		spread_xhair_col.setup( XOR( "visualize spread color" ), XOR( "spread_xhair_col" ), colors::burgundy, nullptr );
		spread_xhair_col.AddShowCallback(callbacks::spread_xhair_enabled);

		RegisterElement( &spread_xhair_col, 1 );

		pen_crosshair.setup( XOR( "penetration crosshair" ), XOR( "pen_xhair" ) );
		RegisterElement( &pen_crosshair, 1 );

		indicators.setup( XOR( "indicators" ), XOR( "indicators" ), { XOR( "lby" ), XOR( "lag compensation" ), XOR( "fake latency" ), XOR( "damage override" ), XOR( "force body aim" ) } );
		RegisterElement( &indicators, 1 );

		tracers.setup( XOR( "grenade simulation" ), XOR( "tracers" ) );
		RegisterElement( &tracers, 1 );

		grenade_prediction_color.setup( XOR( "grenade simulation color" ), XOR( "grenade_prediction_color" ), colors::light_blue, nullptr );
		grenade_prediction_color.AddShowCallback(callbacks::grenade_prediction_vis);
		RegisterElement( &grenade_prediction_color, 1 );

		grenade_trajectory.setup( XOR( "grenade trajectory" ), XOR( "grenade_trajectory" ) );
		RegisterElement( &grenade_trajectory, 1 );

		grenade_trajectory_color.setup( XOR( "grenade trajectory color" ), XOR( "grenade_trajectory_color" ), colors::white, nullptr );
		grenade_trajectory_color.AddShowCallback(callbacks::grenade_trajectoryenabled);
		RegisterElement( &grenade_trajectory_color, 1 );

		impact_beams.setup( XOR( "impact beams" ), XOR( "impact_beams" ) );
		RegisterElement( &impact_beams, 1 );

		impact_beams_color.setup( XOR( "impact beams color" ), XOR( "impact_beams_color" ), colors::light_blue );
		impact_beams_color.AddShowCallback(callbacks::impact_beams_enabled);

		RegisterElement( &impact_beams_color, 1 );

		impact_beams_hurt_color.setup( XOR( "impact beams hurt color" ), XOR( "impact_beams_hurt_color" ), colors::red );
		impact_beams_hurt_color.AddShowCallback(callbacks::impact_beams_enabled);

		RegisterElement( &impact_beams_hurt_color, 1 );

		impact_beams_time.setup( XOR( "impact beams time" ), XOR( "impact_beams_time" ), 1.f, 10.f, true, 0, 1.f, 1.f, XOR( L"s" ) );
		impact_beams_time.AddShowCallback(callbacks::impact_beams_enabled);

		RegisterElement( &impact_beams_time, 1 );

		//local_aa_lines.setup( XOR( "angle lines" ), XOR( "local_aa_lines" ), { XOR( "real" ), XOR( "lby" ) } );
		//RegisterElement( &local_aa_lines, 1 );

		//color_real.setup( XOR( "real color" ), XOR( "color_real" ), colors::green );
		//RegisterElement( &color_real, 1 );

		//color_lby.setup( XOR( "lby color" ), XOR( "color_lby" ), colors::red );
		//RegisterElement( &color_lby, 1 );

		thirdperson.setup( XOR( "thirdperson" ), XOR( "thirdperson" ) );
		thirdperson.SetToggleCallback( callbacks::ToggleThirdPerson );
		RegisterElement( &thirdperson, 1 );

		thirdperson_distance.setup( "", XOR( "thirdperson_distance" ), 60.f, 150.f, false, 0, 150.f, 1.f );
		thirdperson_distance.AddShowCallback(callbacks::thirdpersonbind);
		RegisterElement( &thirdperson_distance, 1 );
	}
};

class MovementTab : public Tab
{
public:
	Checkbox bhop;
	Checkbox airduck;
	Checkbox quickstop;
	Checkbox autostrafe;
	Checkbox slidewalk;

	Keybind  fakewalk;
	Keybind  autopeek;
	Colorpicker autopeek_color;

public:
	void init( )
	{
		SetTitle( XOR( "movement" ) );

		bhop.setup( XOR( "automatic jump" ), XOR( "bhop" ) );
		RegisterElement( &bhop );

		airduck.setup( XOR( "duck in air" ), XOR( "airduck" ) );
		RegisterElement( &airduck );

		quickstop.setup( XOR( "quick stop" ), XOR( "quickstop" ) );
		RegisterElement( &quickstop );

		autostrafe.setup( XOR( "automatic strafe" ), XOR( "autostrafe" ) );
		RegisterElement( &autostrafe );

		slidewalk.setup( XOR( "slide walk" ), XOR( "slidewalk" ) );
		RegisterElement( &slidewalk );

		fakewalk.setup( XOR( "fake-walk" ), XOR( "fakewalk" ) );
		RegisterElement( &fakewalk, 1 );

		autopeek.setup( XOR( "quick peek assist" ), XOR( "autopeek" ) );
		RegisterElement( &autopeek, 1 );

		autopeek_color.setup( XOR( "quick peek assist color" ), XOR( "autopeek_color" ), colors::orange, nullptr );
		RegisterElement( &autopeek_color, 1 );
	}
};

class SkinsTab : public Tab {
public:
	Checkbox enable;
	Checkbox custom_paintkit;
	Checkbox stickers;
	Slider stickers1;
	Slider stickers2;
	Slider stickers3;
	Slider stickers4;


	Edit     id_deagle;
	Checkbox stattrak_deagle;
	Slider   quality_deagle;
	Slider	 seed_deagle;


	Edit     id_elite;
	Checkbox stattrak_elite;
	Slider   quality_elite;
	Slider	 seed_elite;


	Edit     id_fiveseven;
	Checkbox stattrak_fiveseven;
	Slider   quality_fiveseven;
	Slider	 seed_fiveseven;



	Edit     id_glock;
	Checkbox stattrak_glock;
	Slider   quality_glock;
	Slider	 seed_glock;


	Edit     id_ak47;
	Checkbox stattrak_ak47;
	Slider   quality_ak47;
	Slider	 seed_ak47;



	Edit     id_aug;
	Checkbox stattrak_aug;
	Slider   quality_aug;
	Slider	 seed_aug;


	Edit     id_awp;
	Checkbox stattrak_awp;
	Slider   quality_awp;
	Slider	 seed_awp;



	Edit     id_famas;
	Checkbox stattrak_famas;
	Slider   quality_famas;
	Slider	 seed_famas;

	Edit     id_g3sg1;
	Checkbox stattrak_g3sg1;
	Slider   quality_g3sg1;
	Slider	 seed_g3sg1;

	Edit     id_galil;
	Checkbox stattrak_galil;
	Slider   quality_galil;
	Slider	 seed_galil;


	Edit     id_m249;
	Checkbox stattrak_m249;
	Slider   quality_m249;
	Slider	 seed_m249;



	Edit     id_m4a4;
	Checkbox stattrak_m4a4;
	Slider   quality_m4a4;
	Slider	 seed_m4a4;

	Edit     id_mac10;
	Checkbox stattrak_mac10;
	Slider   quality_mac10;
	Slider	 seed_mac10;


	Edit     id_p90;
	Checkbox stattrak_p90;
	Slider   quality_p90;
	Slider	 seed_p90;


	Edit     id_ump45;
	Checkbox stattrak_ump45;
	Slider   quality_ump45;
	Slider	 seed_ump45;



	Edit     id_xm1014;
	Checkbox stattrak_xm1014;
	Slider   quality_xm1014;
	Slider	 seed_xm1014;


	Edit     id_bizon;
	Checkbox stattrak_bizon;
	Slider   quality_bizon;
	Slider	 seed_bizon;



	Edit     id_mag7;
	Checkbox stattrak_mag7;
	Slider   quality_mag7;
	Slider	 seed_mag7;



	Edit     id_negev;
	Checkbox stattrak_negev;
	Slider   quality_negev;
	Slider	 seed_negev;


	Edit     id_sawedoff;
	Checkbox stattrak_sawedoff;
	Slider   quality_sawedoff;
	Slider	 seed_sawedoff;



	Edit     id_tec9;
	Checkbox stattrak_tec9;
	Slider   quality_tec9;
	Slider	 seed_tec9;



	Edit     id_p2000;
	Checkbox stattrak_p2000;
	Slider   quality_p2000;
	Slider	 seed_p2000;



	Edit     id_mp7;
	Checkbox stattrak_mp7;
	Slider   quality_mp7;
	Slider	 seed_mp7;



	Edit     id_mp9;
	Checkbox stattrak_mp9;
	Slider   quality_mp9;
	Slider	 seed_mp9;


	Edit     id_nova;
	Checkbox stattrak_nova;
	Slider   quality_nova;
	Slider	 seed_nova;


	Edit     id_p250;
	Checkbox stattrak_p250;
	Slider   quality_p250;
	Slider	 seed_p250;



	Edit     id_scar20;
	Checkbox stattrak_scar20;
	Slider   quality_scar20;
	Slider	 seed_scar20;


	Edit     id_sg553;
	Checkbox stattrak_sg553;
	Slider   quality_sg553;
	Slider	 seed_sg553;


	Edit     id_ssg08;
	Checkbox stattrak_ssg08;
	Slider   quality_ssg08;
	Slider	 seed_ssg08;


	Edit     id_m4a1s;
	Checkbox stattrak_m4a1s;
	Slider   quality_m4a1s;
	Slider	 seed_m4a1s;


	Edit     id_usps;
	Checkbox stattrak_usps;
	Slider   quality_usps;
	Slider	 seed_usps;


	Edit     id_cz75a;
	Checkbox stattrak_cz75a;
	Slider   quality_cz75a;
	Slider	 seed_cz75a;


	Edit     id_revolver;
	Checkbox stattrak_revolver;
	Slider   quality_revolver;
	Slider	 seed_revolver;


	Edit     id_bayonet;
	Checkbox stattrak_bayonet;
	Slider   quality_bayonet;
	Slider	 seed_bayonet;



	Edit     id_flip;
	Checkbox stattrak_flip;
	Slider   quality_flip;
	Slider	 seed_flip;

	Edit     id_gut;
	Checkbox stattrak_gut;
	Slider   quality_gut;
	Slider	 seed_gut;


	Edit     id_karambit;
	Checkbox stattrak_karambit;
	Slider   quality_karambit;
	Slider	 seed_karambit;



	Edit     id_m9;
	Checkbox stattrak_m9;
	Slider   quality_m9;
	Slider	 seed_m9;


	Edit     id_huntsman;
	Checkbox stattrak_huntsman;
	Slider   quality_huntsman;
	Slider	 seed_huntsman;



	Edit     id_falchion;
	Checkbox stattrak_falchion;
	Slider   quality_falchion;
	Slider	 seed_falchion;

	Edit     id_bowie;
	Checkbox stattrak_bowie;
	Slider   quality_bowie;
	Slider	 seed_bowie;


	Edit     id_butterfly;
	Checkbox stattrak_butterfly;
	Slider   quality_butterfly;
	Slider	 seed_butterfly;



	Edit     id_daggers;
	Checkbox stattrak_daggers;
	Slider   quality_daggers;
	Slider	 seed_daggers;
	Colorpicker color;
	Colorpicker color1;
	Colorpicker color2;
	Colorpicker color3;

	// column 2.
	Dropdown knife;
	Dropdown glove;
	Edit	 glove_id;
	Slider	 shine;
	Slider	 mettalic;
	Slider	 pearlescent;
public:
	void init( ) {
		SetTitle( XOR( "skins" ) );

		enable.setup( XOR( "enable" ), XOR( "skins_enable" ) ); //Main Switch
		enable.SetCallback( callbacks::ForceFullUpdate ); //Force update of CSGO UI
		RegisterElement( &enable );

		// PISTOLS \ USP-S

		id_usps.setup( XOR( "paintkit id" ), XOR( "id_usps" ), 3 );
		id_usps.SetCallback( callbacks::SkinUpdate );
		id_usps.AddShowCallback( callbacks::USPS );
		RegisterElement( &id_usps );

		stattrak_usps.setup( XOR( "stattrak" ), XOR( "stattrak_usps" ) );
		stattrak_usps.SetCallback( callbacks::SkinUpdate );
		stattrak_usps.AddShowCallback( callbacks::USPS );
		RegisterElement( &stattrak_usps );

		quality_usps.setup( XOR( "quality" ), XOR( "quality_usps" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_usps.SetCallback( callbacks::SkinUpdate );
		quality_usps.AddShowCallback( callbacks::USPS );
		RegisterElement( &quality_usps );

		seed_usps.setup( XOR( "seed" ), XOR( "seed_usps" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_usps.SetCallback( callbacks::SkinUpdate );
		seed_usps.AddShowCallback( callbacks::USPS );
		RegisterElement( &seed_usps );



		// PISTOLS \ CZ75-Auto

		id_cz75a.setup( XOR( "paintkit id" ), XOR( "id_cz75a" ), 3 );
		id_cz75a.SetCallback( callbacks::SkinUpdate );
		id_cz75a.AddShowCallback( callbacks::CZ75A );
		RegisterElement( &id_cz75a );

		stattrak_cz75a.setup( XOR( "stattrak" ), XOR( "stattrak_cz75a" ) );
		stattrak_cz75a.SetCallback( callbacks::SkinUpdate );
		stattrak_cz75a.AddShowCallback( callbacks::CZ75A );
		RegisterElement( &stattrak_cz75a );

		quality_cz75a.setup( XOR( "quality" ), XOR( "quality_cz75a" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_cz75a.SetCallback( callbacks::SkinUpdate );
		quality_cz75a.AddShowCallback( callbacks::CZ75A );
		RegisterElement( &quality_cz75a );

		seed_cz75a.setup( XOR( "seed" ), XOR( "seed_cz75a" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_cz75a.SetCallback( callbacks::SkinUpdate );
		seed_cz75a.AddShowCallback( callbacks::CZ75A );
		RegisterElement( &seed_cz75a );



		// PISTOL \ Tec-9

		id_tec9.setup( XOR( "paintkit id" ), XOR( "id_tec9" ), 3 );
		id_tec9.SetCallback( callbacks::SkinUpdate );
		id_tec9.AddShowCallback( callbacks::TEC9 );
		RegisterElement( &id_tec9 );

		stattrak_tec9.setup( XOR( "stattrak" ), XOR( "stattrak_tec9" ) );
		stattrak_tec9.SetCallback( callbacks::SkinUpdate );
		stattrak_tec9.AddShowCallback( callbacks::TEC9 );
		RegisterElement( &stattrak_tec9 );

		quality_tec9.setup( XOR( "quality" ), XOR( "quality_tec9" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_tec9.SetCallback( callbacks::SkinUpdate );
		quality_tec9.AddShowCallback( callbacks::TEC9 );
		RegisterElement( &quality_tec9 );

		seed_tec9.setup( XOR( "seed" ), XOR( "seed_tec9" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_tec9.SetCallback( callbacks::SkinUpdate );
		seed_tec9.AddShowCallback( callbacks::TEC9 );
		RegisterElement( &seed_tec9 );



		// PISTOLS \ P2000

		id_p2000.setup( XOR( "paintkit id" ), XOR( "id_p2000" ), 3 );
		id_p2000.SetCallback( callbacks::SkinUpdate );
		id_p2000.AddShowCallback( callbacks::P2000 );
		RegisterElement( &id_p2000 );

		stattrak_p2000.setup( XOR( "stattrak" ), XOR( "stattrak_p2000" ) );
		stattrak_p2000.SetCallback( callbacks::SkinUpdate );
		stattrak_p2000.AddShowCallback( callbacks::P2000 );
		RegisterElement( &stattrak_p2000 );

		quality_p2000.setup( XOR( "quality" ), XOR( "quality_p2000" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_p2000.SetCallback( callbacks::SkinUpdate );
		quality_p2000.AddShowCallback( callbacks::P2000 );
		RegisterElement( &quality_p2000 );

		seed_p2000.setup( XOR( "seed" ), XOR( "seed_p2000" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_p2000.SetCallback( callbacks::SkinUpdate );
		seed_p2000.AddShowCallback( callbacks::P2000 );
		RegisterElement( &seed_p2000 );



		// PISTOLS \ R8 Revolver

		id_revolver.setup( XOR( "paintkit id" ), XOR( "id_revolver" ), 3 );
		id_revolver.SetCallback( callbacks::SkinUpdate );
		id_revolver.AddShowCallback( callbacks::REVOLVER );
		RegisterElement( &id_revolver );

		stattrak_revolver.setup( XOR( "stattrak" ), XOR( "stattrak_revolver" ) );
		stattrak_revolver.SetCallback( callbacks::SkinUpdate );
		stattrak_revolver.AddShowCallback( callbacks::REVOLVER );
		RegisterElement( &stattrak_revolver );

		quality_revolver.setup( XOR( "quality" ), XOR( "quality_revolver" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_revolver.SetCallback( callbacks::SkinUpdate );
		quality_revolver.AddShowCallback( callbacks::REVOLVER );
		RegisterElement( &quality_revolver );

		seed_revolver.setup( XOR( "seed" ), XOR( "seed_revolver" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_revolver.SetCallback( callbacks::SkinUpdate );
		seed_revolver.AddShowCallback( callbacks::REVOLVER );
		RegisterElement( &seed_revolver );


		// PISTOLS \ Desert Eagle
		id_deagle.setup( XOR( "paintkit id" ), XOR( "id_deagle" ), 3 );
		id_deagle.SetCallback( callbacks::SkinUpdate );
		id_deagle.AddShowCallback( callbacks::DEAGLE );
		RegisterElement( &id_deagle );

		stattrak_deagle.setup( XOR( "stattrak" ), XOR( "stattrak_deagle" ) );
		stattrak_deagle.SetCallback( callbacks::SkinUpdate );
		stattrak_deagle.AddShowCallback( callbacks::DEAGLE );
		RegisterElement( &stattrak_deagle );

		quality_deagle.setup( XOR( "quality" ), XOR( "quality_deagle" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_deagle.SetCallback( callbacks::SkinUpdate );
		quality_deagle.AddShowCallback( callbacks::DEAGLE );
		RegisterElement( &quality_deagle );

		seed_deagle.setup( XOR( "seed" ), XOR( "seed_deagle" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_deagle.SetCallback( callbacks::SkinUpdate );
		seed_deagle.AddShowCallback( callbacks::DEAGLE );
		RegisterElement( &seed_deagle );



		// PISTOLS \ Dual Berettas

		id_elite.setup( XOR( "paintkit id" ), XOR( "id_elite" ), 3 );
		id_elite.SetCallback( callbacks::SkinUpdate );
		id_elite.AddShowCallback( callbacks::ELITE );
		RegisterElement( &id_elite );

		stattrak_elite.setup( XOR( "stattrak" ), XOR( "stattrak_elite" ) );
		stattrak_elite.SetCallback( callbacks::SkinUpdate );
		stattrak_elite.AddShowCallback( callbacks::ELITE );
		RegisterElement( &stattrak_elite );

		quality_elite.setup( XOR( "quality" ), XOR( "quality_elite" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_elite.SetCallback( callbacks::SkinUpdate );
		quality_elite.AddShowCallback( callbacks::ELITE );
		RegisterElement( &quality_elite );

		seed_elite.setup( XOR( "seed" ), XOR( "seed_elite" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_elite.SetCallback( callbacks::SkinUpdate );
		seed_elite.AddShowCallback( callbacks::ELITE );
		RegisterElement( &seed_elite );


		// PISTOLS \ Five-SeveN

		id_fiveseven.setup( XOR( "paintkit id" ), XOR( "id_fiveseven" ), 3 );
		id_fiveseven.SetCallback( callbacks::SkinUpdate );
		id_fiveseven.AddShowCallback( callbacks::FIVESEVEN );
		RegisterElement( &id_fiveseven );

		stattrak_fiveseven.setup( XOR( "stattrak" ), XOR( "stattrak_fiveseven" ) );
		stattrak_fiveseven.SetCallback( callbacks::SkinUpdate );
		stattrak_fiveseven.AddShowCallback( callbacks::FIVESEVEN );
		RegisterElement( &stattrak_fiveseven );

		quality_fiveseven.setup( XOR( "quality" ), XOR( "quality_fiveseven" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_fiveseven.SetCallback( callbacks::SkinUpdate );
		quality_fiveseven.AddShowCallback( callbacks::FIVESEVEN );
		RegisterElement( &quality_fiveseven );

		seed_fiveseven.setup( XOR( "seed" ), XOR( "seed_fiveseven" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_fiveseven.SetCallback( callbacks::SkinUpdate );
		seed_fiveseven.AddShowCallback( callbacks::FIVESEVEN );
		RegisterElement( &seed_fiveseven );


		// PISTOLS \ Glock-18

		id_glock.setup( XOR( "paintkit id" ), XOR( "id_glock" ), 3 );
		id_glock.SetCallback( callbacks::SkinUpdate );
		id_glock.AddShowCallback( callbacks::GLOCK );
		RegisterElement( &id_glock );

		stattrak_glock.setup( XOR( "stattrak" ), XOR( "stattrak_glock" ) );
		stattrak_glock.SetCallback( callbacks::SkinUpdate );
		stattrak_glock.AddShowCallback( callbacks::GLOCK );
		RegisterElement( &stattrak_glock );

		quality_glock.setup( XOR( "quality" ), XOR( "quality_glock" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_glock.SetCallback( callbacks::SkinUpdate );
		quality_glock.AddShowCallback( callbacks::GLOCK );
		RegisterElement( &quality_glock );

		seed_glock.setup( XOR( "seed" ), XOR( "seed_glock" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_glock.SetCallback( callbacks::SkinUpdate );
		seed_glock.AddShowCallback( callbacks::GLOCK );
		RegisterElement( &seed_glock );



		// RIFLES \ AK-47
		id_ak47.setup( XOR( "paintkit id" ), XOR( "id_ak47" ), 3 );
		id_ak47.SetCallback( callbacks::SkinUpdate );
		id_ak47.AddShowCallback( callbacks::AK47 );
		RegisterElement( &id_ak47 );

		stattrak_ak47.setup( XOR( "stattrak" ), XOR( "stattrak_ak47" ) );
		stattrak_ak47.SetCallback( callbacks::SkinUpdate );
		stattrak_ak47.AddShowCallback( callbacks::AK47 );
		RegisterElement( &stattrak_ak47 );

		quality_ak47.setup( XOR( "quality" ), XOR( "quality_ak47" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_ak47.SetCallback( callbacks::SkinUpdate );
		quality_ak47.AddShowCallback( callbacks::AK47 );
		RegisterElement( &quality_ak47 );

		seed_ak47.setup( XOR( "seed" ), XOR( "seed_ak47" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_ak47.SetCallback( callbacks::SkinUpdate );
		seed_ak47.AddShowCallback( callbacks::AK47 );
		RegisterElement( &seed_ak47 );



		// RIFLES \ AUG
		id_aug.setup( XOR( "paintkit id" ), XOR( "id_aug" ), 3 );
		id_aug.SetCallback( callbacks::SkinUpdate );
		id_aug.AddShowCallback( callbacks::AUG );
		RegisterElement( &id_aug );

		stattrak_aug.setup( XOR( "stattrak" ), XOR( "stattrak_aug" ) );
		stattrak_aug.SetCallback( callbacks::SkinUpdate );
		stattrak_aug.AddShowCallback( callbacks::AUG );
		RegisterElement( &stattrak_aug );

		quality_aug.setup( XOR( "quality" ), XOR( "quality_aug" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_aug.SetCallback( callbacks::SkinUpdate );
		quality_aug.AddShowCallback( callbacks::AUG );
		RegisterElement( &quality_aug );

		seed_aug.setup( XOR( "seed" ), XOR( "seed_aug" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_aug.SetCallback( callbacks::SkinUpdate );
		seed_aug.AddShowCallback( callbacks::AUG );
		RegisterElement( &seed_aug );


		//SNIPERS \ AWP

		id_awp.setup( XOR( "paintkit id" ), XOR( "id_awp" ), 3 );
		id_awp.SetCallback( callbacks::SkinUpdate );
		id_awp.AddShowCallback( callbacks::AWP );
		RegisterElement( &id_awp );

		stattrak_awp.setup( XOR( "stattrak" ), XOR( "stattrak_awp" ) );
		stattrak_awp.SetCallback( callbacks::SkinUpdate );
		stattrak_awp.AddShowCallback( callbacks::AWP );
		RegisterElement( &stattrak_awp );

		quality_awp.setup( XOR( "quality" ), XOR( "quality_awp" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_awp.SetCallback( callbacks::SkinUpdate );
		quality_awp.AddShowCallback( callbacks::AWP );
		RegisterElement( &quality_awp );

		seed_awp.setup( XOR( "seed" ), XOR( "seed_awp" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_awp.SetCallback( callbacks::SkinUpdate );
		seed_awp.AddShowCallback( callbacks::AWP );
		RegisterElement( &seed_awp );



		// RIFLES \ FAMAS

		id_famas.setup( XOR( "paintkit id" ), XOR( "id_famas" ), 3 );
		id_famas.SetCallback( callbacks::SkinUpdate );
		id_famas.AddShowCallback( callbacks::FAMAS );
		RegisterElement( &id_famas );

		stattrak_famas.setup( XOR( "stattrak" ), XOR( "stattrak_famas" ) );
		stattrak_famas.SetCallback( callbacks::SkinUpdate );
		stattrak_famas.AddShowCallback( callbacks::FAMAS );
		RegisterElement( &stattrak_famas );

		quality_famas.setup( XOR( "quality" ), XOR( "quality_famas" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_famas.SetCallback( callbacks::SkinUpdate );
		quality_famas.AddShowCallback( callbacks::FAMAS );
		RegisterElement( &quality_famas );

		seed_famas.setup( XOR( "seed" ), XOR( "seed_famas" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_famas.SetCallback( callbacks::SkinUpdate );
		seed_famas.AddShowCallback( callbacks::FAMAS );
		RegisterElement( &seed_famas );



		// SNIPERS \ G3SG1

		id_g3sg1.setup( XOR( "paintkit id" ), XOR( "id_g3sg1" ), 3 );
		id_g3sg1.SetCallback( callbacks::SkinUpdate );
		id_g3sg1.AddShowCallback( callbacks::G3SG1 );
		RegisterElement( &id_g3sg1 );

		stattrak_g3sg1.setup( XOR( "stattrak" ), XOR( "stattrak_g3sg1" ) );
		stattrak_g3sg1.SetCallback( callbacks::SkinUpdate );
		stattrak_g3sg1.AddShowCallback( callbacks::G3SG1 );
		RegisterElement( &stattrak_g3sg1 );

		quality_g3sg1.setup( XOR( "quality" ), XOR( "quality_g3sg1" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_g3sg1.SetCallback( callbacks::SkinUpdate );
		quality_g3sg1.AddShowCallback( callbacks::G3SG1 );
		RegisterElement( &quality_g3sg1 );

		seed_g3sg1.setup( XOR( "seed" ), XOR( "seed_g3sg1" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_g3sg1.SetCallback( callbacks::SkinUpdate );
		seed_g3sg1.AddShowCallback( callbacks::G3SG1 );
		RegisterElement( &seed_g3sg1 );


		// RIFLES \ Galil AR

		id_galil.setup( XOR( "paintkit id" ), XOR( "id_galil" ), 3 );
		id_galil.SetCallback( callbacks::SkinUpdate );
		id_galil.AddShowCallback( callbacks::GALIL );
		RegisterElement( &id_galil );

		stattrak_galil.setup( XOR( "stattrak" ), XOR( "stattrak_galil" ) );
		stattrak_galil.SetCallback( callbacks::SkinUpdate );
		stattrak_galil.AddShowCallback( callbacks::GALIL );
		RegisterElement( &stattrak_galil );

		quality_galil.setup( XOR( "quality" ), XOR( "quality_galil" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_galil.SetCallback( callbacks::SkinUpdate );
		quality_galil.AddShowCallback( callbacks::GALIL );
		RegisterElement( &quality_galil );

		seed_galil.setup( XOR( "seed" ), XOR( "seed_galil" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_galil.SetCallback( callbacks::SkinUpdate );
		seed_galil.AddShowCallback( callbacks::GALIL );
		RegisterElement( &seed_galil );


		// HEAVY \ M249

		id_m249.setup( XOR( "paintkit id" ), XOR( "id_m249" ), 3 );
		id_m249.SetCallback( callbacks::SkinUpdate );
		id_m249.AddShowCallback( callbacks::M249 );
		RegisterElement( &id_m249 );

		stattrak_m249.setup( XOR( "stattrak" ), XOR( "stattrak_m249" ) );
		stattrak_m249.SetCallback( callbacks::SkinUpdate );
		stattrak_m249.AddShowCallback( callbacks::M249 );
		RegisterElement( &stattrak_m249 );

		quality_m249.setup( XOR( "quality" ), XOR( "quality_m249" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_m249.SetCallback( callbacks::SkinUpdate );
		quality_m249.AddShowCallback( callbacks::M249 );
		RegisterElement( &quality_m249 );

		seed_m249.setup( XOR( "seed" ), XOR( "seed_m249" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_m249.SetCallback( callbacks::SkinUpdate );
		seed_m249.AddShowCallback( callbacks::M249 );
		RegisterElement( &seed_m249 );



		// RIFLES \ M4A4

		id_m4a4.setup( XOR( "paintkit id" ), XOR( "id_m4a4" ), 3 );
		id_m4a4.SetCallback( callbacks::SkinUpdate );
		id_m4a4.AddShowCallback( callbacks::M4A4 );
		RegisterElement( &id_m4a4 );

		stattrak_m4a4.setup( XOR( "stattrak" ), XOR( "stattrak_m4a4" ) );
		stattrak_m4a4.SetCallback( callbacks::SkinUpdate );
		stattrak_m4a4.AddShowCallback( callbacks::M4A4 );
		RegisterElement( &stattrak_m4a4 );

		quality_m4a4.setup( XOR( "quality" ), XOR( "quality_m4a4" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_m4a4.SetCallback( callbacks::SkinUpdate );
		quality_m4a4.AddShowCallback( callbacks::M4A4 );
		RegisterElement( &quality_m4a4 );

		seed_m4a4.setup( XOR( "seed" ), XOR( "seed_m4a4" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_m4a4.SetCallback( callbacks::SkinUpdate );
		seed_m4a4.AddShowCallback( callbacks::M4A4 );
		RegisterElement( &seed_m4a4 );


		// SMG \ MAC-10

		id_mac10.setup( XOR( "paintkit id" ), XOR( "id_mac10" ), 3 );
		id_mac10.SetCallback( callbacks::SkinUpdate );
		id_mac10.AddShowCallback( callbacks::MAC10 );
		RegisterElement( &id_mac10 );

		stattrak_mac10.setup( XOR( "stattrak" ), XOR( "stattrak_mac10" ) );
		stattrak_mac10.SetCallback( callbacks::SkinUpdate );
		stattrak_mac10.AddShowCallback( callbacks::MAC10 );
		RegisterElement( &stattrak_mac10 );

		quality_mac10.setup( XOR( "quality" ), XOR( "quality_mac10" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_mac10.SetCallback( callbacks::SkinUpdate );
		quality_mac10.AddShowCallback( callbacks::MAC10 );
		RegisterElement( &quality_mac10 );

		seed_mac10.setup( XOR( "seed" ), XOR( "seed_mac10" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_mac10.SetCallback( callbacks::SkinUpdate );
		seed_mac10.AddShowCallback( callbacks::MAC10 );
		RegisterElement( &seed_mac10 );


		// SMG \ P90

		id_p90.setup( XOR( "paintkit id" ), XOR( "id_p90" ), 3 );
		id_p90.SetCallback( callbacks::SkinUpdate );
		id_p90.AddShowCallback( callbacks::P90 );
		RegisterElement( &id_p90 );

		stattrak_p90.setup( XOR( "stattrak" ), XOR( "stattrak_p90" ) );
		stattrak_p90.SetCallback( callbacks::SkinUpdate );
		stattrak_p90.AddShowCallback( callbacks::P90 );
		RegisterElement( &stattrak_p90 );

		quality_p90.setup( XOR( "quality" ), XOR( "quality_p90" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_p90.SetCallback( callbacks::SkinUpdate );
		quality_p90.AddShowCallback( callbacks::P90 );
		RegisterElement( &quality_p90 );

		seed_p90.setup( XOR( "seed" ), XOR( "seed_p90" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_p90.SetCallback( callbacks::SkinUpdate );
		seed_p90.AddShowCallback( callbacks::P90 );
		RegisterElement( &seed_p90 );


		// SMG \ UMP-45

		id_ump45.setup( XOR( "paintkit id" ), XOR( "id_ump45" ), 3 );
		id_ump45.SetCallback( callbacks::SkinUpdate );
		id_ump45.AddShowCallback( callbacks::UMP45 );
		RegisterElement( &id_ump45 );

		stattrak_ump45.setup( XOR( "stattrak" ), XOR( "stattrak_ump45" ) );
		stattrak_ump45.SetCallback( callbacks::SkinUpdate );
		stattrak_ump45.AddShowCallback( callbacks::UMP45 );
		RegisterElement( &stattrak_ump45 );

		quality_ump45.setup( XOR( "quality" ), XOR( "quality_ump45" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_ump45.SetCallback( callbacks::SkinUpdate );
		quality_ump45.AddShowCallback( callbacks::UMP45 );
		RegisterElement( &quality_ump45 );

		seed_ump45.setup( XOR( "seed" ), XOR( "seed_ump45" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_ump45.SetCallback( callbacks::SkinUpdate );
		seed_ump45.AddShowCallback( callbacks::UMP45 );
		RegisterElement( &seed_ump45 );


		// HEAVY \ XM1014

		id_xm1014.setup( XOR( "paintkit id" ), XOR( "id_xm1014" ), 3 );
		id_xm1014.SetCallback( callbacks::SkinUpdate );
		id_xm1014.AddShowCallback( callbacks::XM1014 );
		RegisterElement( &id_xm1014 );

		stattrak_xm1014.setup( XOR( "stattrak" ), XOR( "stattrak_xm1014" ) );
		stattrak_xm1014.SetCallback( callbacks::SkinUpdate );
		stattrak_xm1014.AddShowCallback( callbacks::XM1014 );
		RegisterElement( &stattrak_xm1014 );

		quality_xm1014.setup( XOR( "quality" ), XOR( "quality_xm1014" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_xm1014.SetCallback( callbacks::SkinUpdate );
		quality_xm1014.AddShowCallback( callbacks::XM1014 );
		RegisterElement( &quality_xm1014 );

		seed_xm1014.setup( XOR( "seed" ), XOR( "seed_xm1014" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_xm1014.SetCallback( callbacks::SkinUpdate );
		seed_xm1014.AddShowCallback( callbacks::XM1014 );
		RegisterElement( &seed_xm1014 );



		// SMG \ PP-Bizon

		id_bizon.setup( XOR( "paintkit id" ), XOR( "id_bizon" ), 3 );
		id_bizon.SetCallback( callbacks::SkinUpdate );
		id_bizon.AddShowCallback( callbacks::BIZON );
		RegisterElement( &id_bizon );

		stattrak_bizon.setup( XOR( "stattrak" ), XOR( "stattrak_bizon" ) );
		stattrak_bizon.SetCallback( callbacks::SkinUpdate );
		stattrak_bizon.AddShowCallback( callbacks::BIZON );
		RegisterElement( &stattrak_bizon );

		quality_bizon.setup( XOR( "quality" ), XOR( "quality_bizon" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_bizon.SetCallback( callbacks::SkinUpdate );
		quality_bizon.AddShowCallback( callbacks::BIZON );
		RegisterElement( &quality_bizon );

		seed_bizon.setup( XOR( "seed" ), XOR( "seed_bizon" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_bizon.SetCallback( callbacks::SkinUpdate );
		seed_bizon.AddShowCallback( callbacks::BIZON );
		RegisterElement( &seed_bizon );



		// HEAVY \ MAG-7

		id_mag7.setup( XOR( "paintkit id" ), XOR( "id_mag7" ), 3 );
		id_mag7.SetCallback( callbacks::SkinUpdate );
		id_mag7.AddShowCallback( callbacks::MAG7 );
		RegisterElement( &id_mag7 );

		stattrak_mag7.setup( XOR( "stattrak" ), XOR( "stattrak_mag7" ) );
		stattrak_mag7.SetCallback( callbacks::SkinUpdate );
		stattrak_mag7.AddShowCallback( callbacks::MAG7 );
		RegisterElement( &stattrak_mag7 );

		quality_mag7.setup( XOR( "quality" ), XOR( "quality_mag7" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_mag7.SetCallback( callbacks::SkinUpdate );
		quality_mag7.AddShowCallback( callbacks::MAG7 );
		RegisterElement( &quality_mag7 );

		seed_mag7.setup( XOR( "seed" ), XOR( "seed_mag7" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_mag7.SetCallback( callbacks::SkinUpdate );
		seed_mag7.AddShowCallback( callbacks::MAG7 );
		RegisterElement( &seed_mag7 );


		//HEAVY \ Negev

		id_negev.setup( XOR( "paintkit id" ), XOR( "id_negev" ), 3 );
		id_negev.SetCallback( callbacks::SkinUpdate );
		id_negev.AddShowCallback( callbacks::NEGEV );
		RegisterElement( &id_negev );

		stattrak_negev.setup( XOR( "stattrak" ), XOR( "stattrak_negev" ) );
		stattrak_negev.SetCallback( callbacks::SkinUpdate );
		stattrak_negev.AddShowCallback( callbacks::NEGEV );
		RegisterElement( &stattrak_negev );

		quality_negev.setup( XOR( "quality" ), XOR( "quality_negev" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_negev.SetCallback( callbacks::SkinUpdate );
		quality_negev.AddShowCallback( callbacks::NEGEV );
		RegisterElement( &quality_negev );

		seed_negev.setup( XOR( "seed" ), XOR( "seed_negev" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_negev.SetCallback( callbacks::SkinUpdate );
		seed_negev.AddShowCallback( callbacks::NEGEV );
		RegisterElement( &seed_negev );



		// HEAVY \ Sawed-Off

		id_sawedoff.setup( XOR( "paintkit id" ), XOR( "id_sawedoff" ), 3 );
		id_sawedoff.SetCallback( callbacks::SkinUpdate );
		id_sawedoff.AddShowCallback( callbacks::SAWEDOFF );
		RegisterElement( &id_sawedoff );

		stattrak_sawedoff.setup( XOR( "stattrak" ), XOR( "stattrak_sawedoff" ) );
		stattrak_sawedoff.SetCallback( callbacks::SkinUpdate );
		stattrak_sawedoff.AddShowCallback( callbacks::SAWEDOFF );
		RegisterElement( &stattrak_sawedoff );

		quality_sawedoff.setup( XOR( "quality" ), XOR( "quality_sawedoff" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_sawedoff.SetCallback( callbacks::SkinUpdate );
		quality_sawedoff.AddShowCallback( callbacks::SAWEDOFF );
		RegisterElement( &quality_sawedoff );

		seed_sawedoff.setup( XOR( "seed" ), XOR( "seed_sawedoff" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_sawedoff.SetCallback( callbacks::SkinUpdate );
		seed_sawedoff.AddShowCallback( callbacks::SAWEDOFF );
		RegisterElement( &seed_sawedoff );

		// SMG \ MP7

		id_mp7.setup( XOR( "paintkit id" ), XOR( "id_mp7" ), 3 );
		id_mp7.SetCallback( callbacks::SkinUpdate );
		id_mp7.AddShowCallback( callbacks::MP7 );
		RegisterElement( &id_mp7 );

		stattrak_mp7.setup( XOR( "stattrak" ), XOR( "stattrak_mp7" ) );
		stattrak_mp7.SetCallback( callbacks::SkinUpdate );
		stattrak_mp7.AddShowCallback( callbacks::MP7 );
		RegisterElement( &stattrak_mp7 );

		quality_mp7.setup( XOR( "quality" ), XOR( "quality_mp7" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_mp7.SetCallback( callbacks::SkinUpdate );
		quality_mp7.AddShowCallback( callbacks::MP7 );
		RegisterElement( &quality_mp7 );

		seed_mp7.setup( XOR( "seed" ), XOR( "seed_mp7" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_mp7.SetCallback( callbacks::SkinUpdate );
		seed_mp7.AddShowCallback( callbacks::MP7 );
		RegisterElement( &seed_mp7 );


		// SMG \ MP9

		id_mp9.setup( XOR( "paintkit id" ), XOR( "id_mp9" ), 3 );
		id_mp9.SetCallback( callbacks::SkinUpdate );
		id_mp9.AddShowCallback( callbacks::MP9 );
		RegisterElement( &id_mp9 );

		stattrak_mp9.setup( XOR( "stattrak" ), XOR( "stattrak_mp9" ) );
		stattrak_mp9.SetCallback( callbacks::SkinUpdate );
		stattrak_mp9.AddShowCallback( callbacks::MP9 );
		RegisterElement( &stattrak_mp9 );

		quality_mp9.setup( XOR( "quality" ), XOR( "quality_mp9" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_mp9.SetCallback( callbacks::SkinUpdate );
		quality_mp9.AddShowCallback( callbacks::MP9 );
		RegisterElement( &quality_mp9 );

		seed_mp9.setup( XOR( "seed" ), XOR( "seed_mp9" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_mp9.SetCallback( callbacks::SkinUpdate );
		seed_mp9.AddShowCallback( callbacks::MP9 );
		RegisterElement( &seed_mp9 );



		// HEAVY \ Nova

		id_nova.setup( XOR( "paintkit id" ), XOR( "id_nova" ), 3 );
		id_nova.SetCallback( callbacks::SkinUpdate );
		id_nova.AddShowCallback( callbacks::NOVA );
		RegisterElement( &id_nova );

		stattrak_nova.setup( XOR( "stattrak" ), XOR( "stattrak_nova" ) );
		stattrak_nova.SetCallback( callbacks::SkinUpdate );
		stattrak_nova.AddShowCallback( callbacks::NOVA );
		RegisterElement( &stattrak_nova );

		quality_nova.setup( XOR( "quality" ), XOR( "quality_nova" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_nova.SetCallback( callbacks::SkinUpdate );
		quality_nova.AddShowCallback( callbacks::NOVA );
		RegisterElement( &quality_nova );

		seed_nova.setup( XOR( "seed" ), XOR( "seed_nova" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_nova.SetCallback( callbacks::SkinUpdate );
		seed_nova.AddShowCallback( callbacks::NOVA );
		RegisterElement( &seed_nova );


		// PISTOLS \ P250

		id_p250.setup( XOR( "paintkit id" ), XOR( "id_p250" ), 3 );
		id_p250.SetCallback( callbacks::SkinUpdate );
		id_p250.AddShowCallback( callbacks::P250 );
		RegisterElement( &id_p250 );

		stattrak_p250.setup( XOR( "stattrak" ), XOR( "stattrak_p250" ) );
		stattrak_p250.SetCallback( callbacks::SkinUpdate );
		stattrak_p250.AddShowCallback( callbacks::P250 );
		RegisterElement( &stattrak_p250 );

		quality_p250.setup( XOR( "quality" ), XOR( "quality_p250" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_p250.SetCallback( callbacks::SkinUpdate );
		quality_p250.AddShowCallback( callbacks::P250 );
		RegisterElement( &quality_p250 );

		seed_p250.setup( XOR( "seed" ), XOR( "seed_p250" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_p250.SetCallback( callbacks::SkinUpdate );
		seed_p250.AddShowCallback( callbacks::P250 );
		RegisterElement( &seed_p250 );



		// SNIPERS \ SCAR-20

		id_scar20.setup( XOR( "paintkit id" ), XOR( "id_scar20" ), 3 );
		id_scar20.SetCallback( callbacks::SkinUpdate );
		id_scar20.AddShowCallback( callbacks::SCAR20 );
		RegisterElement( &id_scar20 );

		stattrak_scar20.setup( XOR( "stattrak" ), XOR( "stattrak_scar20" ) );
		stattrak_scar20.SetCallback( callbacks::SkinUpdate );
		stattrak_scar20.AddShowCallback( callbacks::SCAR20 );
		RegisterElement( &stattrak_scar20 );

		quality_scar20.setup( XOR( "quality" ), XOR( "quality_scar20" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_scar20.SetCallback( callbacks::SkinUpdate );
		quality_scar20.AddShowCallback( callbacks::SCAR20 );
		RegisterElement( &quality_scar20 );

		seed_scar20.setup( XOR( "seed" ), XOR( "seed_scar20" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_scar20.SetCallback( callbacks::SkinUpdate );
		seed_scar20.AddShowCallback( callbacks::SCAR20 );
		RegisterElement( &seed_scar20 );



		// RIFLES \ SG 553

		id_sg553.setup( XOR( "paintkit id" ), XOR( "id_sg553" ), 3 );
		id_sg553.SetCallback( callbacks::SkinUpdate );
		id_sg553.AddShowCallback( callbacks::SG553 );
		RegisterElement( &id_sg553 );

		stattrak_sg553.setup( XOR( "stattrak" ), XOR( "stattrak_sg553" ) );
		stattrak_sg553.SetCallback( callbacks::SkinUpdate );
		stattrak_sg553.AddShowCallback( callbacks::SG553 );
		RegisterElement( &stattrak_sg553 );

		quality_sg553.setup( XOR( "quality" ), XOR( "quality_sg553" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_sg553.SetCallback( callbacks::SkinUpdate );
		quality_sg553.AddShowCallback( callbacks::SG553 );
		RegisterElement( &quality_sg553 );

		seed_sg553.setup( XOR( "seed" ), XOR( "seed_sg553" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_sg553.SetCallback( callbacks::SkinUpdate );
		seed_sg553.AddShowCallback( callbacks::SG553 );
		RegisterElement( &seed_sg553 );


		// SNIPERS \ SSG 08

		id_ssg08.setup( XOR( "paintkit id" ), XOR( "id_ssg08" ), 3 );
		id_ssg08.SetCallback( callbacks::SkinUpdate );
		id_ssg08.AddShowCallback( callbacks::SSG08 );
		RegisterElement( &id_ssg08 );

		stattrak_ssg08.setup( XOR( "stattrak" ), XOR( "stattrak_ssg08" ) );
		stattrak_ssg08.SetCallback( callbacks::SkinUpdate );
		stattrak_ssg08.AddShowCallback( callbacks::SSG08 );
		RegisterElement( &stattrak_ssg08 );

		quality_ssg08.setup( XOR( "quality" ), XOR( "quality_ssg08" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_ssg08.SetCallback( callbacks::SkinUpdate );
		quality_ssg08.AddShowCallback( callbacks::SSG08 );
		RegisterElement( &quality_ssg08 );

		seed_ssg08.setup( XOR( "seed" ), XOR( "seed_ssg08" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_ssg08.SetCallback( callbacks::SkinUpdate );
		seed_ssg08.AddShowCallback( callbacks::SSG08 );
		RegisterElement( &seed_ssg08 );


		// RIFLES \ M4A1-S

		id_m4a1s.setup( XOR( "paintkit id" ), XOR( "id_m4a1s" ), 3 );
		id_m4a1s.SetCallback( callbacks::SkinUpdate );
		id_m4a1s.AddShowCallback( callbacks::M4A1S );
		RegisterElement( &id_m4a1s );

		stattrak_m4a1s.setup( XOR( "stattrak" ), XOR( "stattrak_m4a1s" ) );
		stattrak_m4a1s.SetCallback( callbacks::SkinUpdate );
		stattrak_m4a1s.AddShowCallback( callbacks::M4A1S );
		RegisterElement( &stattrak_m4a1s );

		quality_m4a1s.setup( XOR( "quality" ), XOR( "quality_m4a1s" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_m4a1s.SetCallback( callbacks::SkinUpdate );
		quality_m4a1s.AddShowCallback( callbacks::M4A1S );
		RegisterElement( &quality_m4a1s );

		seed_m4a1s.setup( XOR( "seed" ), XOR( "seed_m4a1s" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_m4a1s.SetCallback( callbacks::SkinUpdate );
		seed_m4a1s.AddShowCallback( callbacks::M4A1S );
		RegisterElement( &seed_m4a1s );


		//ALL DONE WOOHOO

		// KNIVES \ Bayonet

		id_bayonet.setup( XOR( "paintkit id" ), XOR( "id_bayonet" ), 3 );
		id_bayonet.SetCallback( callbacks::SkinUpdate );
		id_bayonet.AddShowCallback( callbacks::KNIFE_BAYONET );
		RegisterElement( &id_bayonet );

		stattrak_bayonet.setup( XOR( "stattrak" ), XOR( "stattrak_bayonet" ) );
		stattrak_bayonet.SetCallback( callbacks::SkinUpdate );
		stattrak_bayonet.AddShowCallback( callbacks::KNIFE_BAYONET );
		RegisterElement( &stattrak_bayonet );

		quality_bayonet.setup( XOR( "quality" ), XOR( "quality_bayonet" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_bayonet.SetCallback( callbacks::SkinUpdate );
		quality_bayonet.AddShowCallback( callbacks::KNIFE_BAYONET );
		RegisterElement( &quality_bayonet );

		seed_bayonet.setup( XOR( "seed" ), XOR( "seed_bayonet" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_bayonet.SetCallback( callbacks::SkinUpdate );
		seed_bayonet.AddShowCallback( callbacks::KNIFE_BAYONET );
		RegisterElement( &seed_bayonet );


		// KNIVES \ Flip Knife

		id_flip.setup( XOR( "paintkit id" ), XOR( "id_flip" ), 3 );
		id_flip.SetCallback( callbacks::SkinUpdate );
		id_flip.AddShowCallback( callbacks::KNIFE_FLIP );
		RegisterElement( &id_flip );

		stattrak_flip.setup( XOR( "stattrak" ), XOR( "stattrak_flip" ) );
		stattrak_flip.SetCallback( callbacks::SkinUpdate );
		stattrak_flip.AddShowCallback( callbacks::KNIFE_FLIP );
		RegisterElement( &stattrak_flip );

		quality_flip.setup( XOR( "quality" ), XOR( "quality_flip" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_flip.SetCallback( callbacks::SkinUpdate );
		quality_flip.AddShowCallback( callbacks::KNIFE_FLIP );
		RegisterElement( &quality_flip );

		seed_flip.setup( XOR( "seed" ), XOR( "seed_flip" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_flip.SetCallback( callbacks::SkinUpdate );
		seed_flip.AddShowCallback( callbacks::KNIFE_FLIP );
		RegisterElement( &seed_flip );


		// KNIVES \ Gut Knife

		id_gut.setup( XOR( "paintkit id" ), XOR( "id_gut" ), 3 );
		id_gut.SetCallback( callbacks::SkinUpdate );
		id_gut.AddShowCallback( callbacks::KNIFE_GUT );
		RegisterElement( &id_gut );

		stattrak_gut.setup( XOR( "stattrak" ), XOR( "stattrak_gut" ) );
		stattrak_gut.SetCallback( callbacks::SkinUpdate );
		stattrak_gut.AddShowCallback( callbacks::KNIFE_GUT );
		RegisterElement( &stattrak_gut );

		quality_gut.setup( XOR( "quality" ), XOR( "quality_gut" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_gut.SetCallback( callbacks::SkinUpdate );
		quality_gut.AddShowCallback( callbacks::KNIFE_GUT );
		RegisterElement( &quality_gut );

		seed_gut.setup( XOR( "seed" ), XOR( "seed_gut" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_gut.SetCallback( callbacks::SkinUpdate );
		seed_gut.AddShowCallback( callbacks::KNIFE_GUT );
		RegisterElement( &seed_gut );


		// KNIVES \ Karambit Knife

		id_karambit.setup( XOR( "paintkit id" ), XOR( "id_karambit" ), 3 );
		id_karambit.SetCallback( callbacks::SkinUpdate );
		id_karambit.AddShowCallback( callbacks::KNIFE_KARAMBIT );
		RegisterElement( &id_karambit );

		stattrak_karambit.setup( XOR( "stattrak" ), XOR( "stattrak_karambit" ) );
		stattrak_karambit.SetCallback( callbacks::SkinUpdate );
		stattrak_karambit.AddShowCallback( callbacks::KNIFE_KARAMBIT );
		RegisterElement( &stattrak_karambit );

		quality_karambit.setup( XOR( "quality" ), XOR( "quality_karambit" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_karambit.SetCallback( callbacks::SkinUpdate );
		quality_karambit.AddShowCallback( callbacks::KNIFE_KARAMBIT );
		RegisterElement( &quality_karambit );

		seed_karambit.setup( XOR( "seed" ), XOR( "seed_karambit" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_karambit.SetCallback( callbacks::SkinUpdate );
		seed_karambit.AddShowCallback( callbacks::KNIFE_KARAMBIT );
		RegisterElement( &seed_karambit );


		// KNIVES \ M9 Bayonet

		id_m9.setup( XOR( "paintkit id" ), XOR( "id_m9" ), 3 );
		id_m9.SetCallback( callbacks::SkinUpdate );
		id_m9.AddShowCallback( callbacks::KNIFE_M9_BAYONET );
		RegisterElement( &id_m9 );

		stattrak_m9.setup( XOR( "stattrak" ), XOR( "stattrak_m9" ) );
		stattrak_m9.SetCallback( callbacks::SkinUpdate );
		stattrak_m9.AddShowCallback( callbacks::KNIFE_M9_BAYONET );
		RegisterElement( &stattrak_m9 );

		quality_m9.setup( XOR( "quality" ), XOR( "quality_m9" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_m9.SetCallback( callbacks::SkinUpdate );
		quality_m9.AddShowCallback( callbacks::KNIFE_M9_BAYONET );
		RegisterElement( &quality_m9 );

		seed_m9.setup( XOR( "seed" ), XOR( "seed_m9" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_m9.SetCallback( callbacks::SkinUpdate );
		seed_m9.AddShowCallback( callbacks::KNIFE_M9_BAYONET );
		RegisterElement( &seed_m9 );



		// KNIVES \ Huntsman Knife

		id_huntsman.setup( XOR( "paintkit id" ), XOR( "id_huntsman" ), 3 );
		id_huntsman.SetCallback( callbacks::SkinUpdate );
		id_huntsman.AddShowCallback( callbacks::KNIFE_HUNTSMAN );
		RegisterElement( &id_huntsman );

		stattrak_huntsman.setup( XOR( "stattrak" ), XOR( "stattrak_huntsman" ) );
		stattrak_huntsman.SetCallback( callbacks::SkinUpdate );
		stattrak_huntsman.AddShowCallback( callbacks::KNIFE_HUNTSMAN );
		RegisterElement( &stattrak_huntsman );

		quality_huntsman.setup( XOR( "quality" ), XOR( "quality_huntsman" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_huntsman.SetCallback( callbacks::SkinUpdate );
		quality_huntsman.AddShowCallback( callbacks::KNIFE_HUNTSMAN );
		RegisterElement( &quality_huntsman );

		seed_huntsman.setup( XOR( "seed" ), XOR( "seed_huntsman" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_huntsman.SetCallback( callbacks::SkinUpdate );
		seed_huntsman.AddShowCallback( callbacks::KNIFE_HUNTSMAN );
		RegisterElement( &seed_huntsman );



		// KNIVES \ Falchion Knife

		id_falchion.setup( XOR( "paintkit id" ), XOR( "id_falchion" ), 3 );
		id_falchion.SetCallback( callbacks::SkinUpdate );
		id_falchion.AddShowCallback( callbacks::KNIFE_FALCHION );
		RegisterElement( &id_falchion );

		stattrak_falchion.setup( XOR( "stattrak" ), XOR( "stattrak_falchion" ) );
		stattrak_falchion.SetCallback( callbacks::SkinUpdate );
		stattrak_falchion.AddShowCallback( callbacks::KNIFE_FALCHION );
		RegisterElement( &stattrak_falchion );

		quality_falchion.setup( XOR( "quality" ), XOR( "quality_falchion" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_falchion.SetCallback( callbacks::SkinUpdate );
		quality_falchion.AddShowCallback( callbacks::KNIFE_FALCHION );
		RegisterElement( &quality_falchion );

		seed_falchion.setup( XOR( "seed" ), XOR( "seed_falchion" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_falchion.SetCallback( callbacks::SkinUpdate );
		seed_falchion.AddShowCallback( callbacks::KNIFE_FALCHION );
		RegisterElement( &seed_falchion );

		// KNIVES \ Bowie Knife

		id_bowie.setup( XOR( "paintkit id" ), XOR( "id_bowie" ), 3 );
		id_bowie.SetCallback( callbacks::SkinUpdate );
		id_bowie.AddShowCallback( callbacks::KNIFE_BOWIE );
		RegisterElement( &id_bowie );

		stattrak_bowie.setup( XOR( "stattrak" ), XOR( "stattrak_bowie" ) );
		stattrak_bowie.SetCallback( callbacks::SkinUpdate );
		stattrak_bowie.AddShowCallback( callbacks::KNIFE_BOWIE );
		RegisterElement( &stattrak_bowie );

		quality_bowie.setup( XOR( "quality" ), XOR( "quality_bowie" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_bowie.SetCallback( callbacks::SkinUpdate );
		quality_bowie.AddShowCallback( callbacks::KNIFE_BOWIE );
		RegisterElement( &quality_bowie );

		seed_bowie.setup( XOR( "seed" ), XOR( "seed_bowie" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_bowie.SetCallback( callbacks::SkinUpdate );
		seed_bowie.AddShowCallback( callbacks::KNIFE_BOWIE );
		RegisterElement( &seed_bowie );


		// KNIVES \ Butterfly Knife

		id_butterfly.setup( XOR( "paintkit id" ), XOR( "id_butterfly" ), 3 );
		id_butterfly.SetCallback( callbacks::SkinUpdate );
		id_butterfly.AddShowCallback( callbacks::KNIFE_BUTTERFLY );
		RegisterElement( &id_butterfly );

		stattrak_butterfly.setup( XOR( "stattrak" ), XOR( "stattrak_butterfly" ) );
		stattrak_butterfly.SetCallback( callbacks::SkinUpdate );
		stattrak_butterfly.AddShowCallback( callbacks::KNIFE_BUTTERFLY );
		RegisterElement( &stattrak_butterfly );

		quality_butterfly.setup( XOR( "quality" ), XOR( "quality_butterfly" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_butterfly.SetCallback( callbacks::SkinUpdate );
		quality_butterfly.AddShowCallback( callbacks::KNIFE_BUTTERFLY );
		RegisterElement( &quality_butterfly );

		seed_butterfly.setup( XOR( "seed" ), XOR( "seed_butterfly" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_butterfly.SetCallback( callbacks::SkinUpdate );
		seed_butterfly.AddShowCallback( callbacks::KNIFE_BUTTERFLY );
		RegisterElement( &seed_butterfly );




		// KNIVES \ Shadow Daggers

		id_daggers.setup( XOR( "paintkit id" ), XOR( "id_daggers" ), 3 );
		id_daggers.SetCallback( callbacks::SkinUpdate );
		id_daggers.AddShowCallback( callbacks::KNIFE_SHADOW_DAGGERS );
		RegisterElement( &id_daggers );

		stattrak_daggers.setup( XOR( "stattrak" ), XOR( "stattrak_daggers" ) );
		stattrak_daggers.SetCallback( callbacks::SkinUpdate );
		stattrak_daggers.AddShowCallback( callbacks::KNIFE_SHADOW_DAGGERS );
		RegisterElement( &stattrak_daggers );

		quality_daggers.setup( XOR( "quality" ), XOR( "quality_daggers" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		quality_daggers.SetCallback( callbacks::SkinUpdate );
		quality_daggers.AddShowCallback( callbacks::KNIFE_SHADOW_DAGGERS );
		RegisterElement( &quality_daggers );

		seed_daggers.setup( XOR( "seed" ), XOR( "seed_daggers" ), 0.f, 255.f, true, 0, 0.f, 1.f );
		seed_daggers.SetCallback( callbacks::SkinUpdate );
		seed_daggers.AddShowCallback( callbacks::KNIFE_SHADOW_DAGGERS );
		RegisterElement( &seed_daggers );

		custom_paintkit.setup( XOR( "paint-kit" ), XOR( "custom_paintkit" ) );
		custom_paintkit.SetCallback( callbacks::ForceFullUpdate );
		RegisterElement( &custom_paintkit );

		color.setup( XOR( "color" ), XOR( "color" ), colors::white );
		color.SetCallback( callbacks::SkinUpdate );
		color.AddShowCallback( callbacks::niggasinparis );
		RegisterElement( &color );


		color1.setup( XOR( "color 1" ), XOR( "color1" ), colors::white );
		color1.SetCallback( callbacks::SkinUpdate );
		color1.AddShowCallback( callbacks::niggasinparis );
		RegisterElement( &color1 );



		color2.setup( XOR( "color 2" ), XOR( "color2" ), colors::white );
		color2.SetCallback( callbacks::SkinUpdate );
		color2.AddShowCallback( callbacks::niggasinparis );
		RegisterElement( &color2 );


		color3.setup( XOR( "color 3" ), XOR( "color3" ), colors::white );
		color3.SetCallback( callbacks::SkinUpdate );
		color3.AddShowCallback( callbacks::niggasinparis );
		RegisterElement( &color3 );

		pearlescent.setup( XOR( "pearlescent" ), XOR( "pearlescent" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		RegisterElement( &pearlescent );

		shine.setup( XOR( "shine" ), XOR( "shine" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		RegisterElement( &shine );

		mettalic.setup( XOR( "mettalic" ), XOR( "mettalic" ), 1.f, 100.f, true, 0, 100.f, 1.f, XOR( L"%" ) );
		RegisterElement( &mettalic );


		// col 2.
		knife.setup( XOR( "knife model" ), XOR( "skins_knife_model" ), { XOR( "off" ), XOR( "bayonet" ), XOR( "bowie" ), XOR( "butterfly" ), XOR( "falchion" ), XOR( "flip" ), XOR( "gut" ), XOR( "huntsman" ), XOR( "karambit" ), XOR( "m9 bayonet" ), XOR( "daggers" ) } );
		knife.SetCallback( callbacks::SkinUpdate );
		RegisterElement( &knife, 1 );

		glove.setup( XOR( "glove model" ), XOR( "skins_glove_model" ), { XOR( "off" ), XOR( "bloodhound" ), XOR( "sport" ), XOR( "driver" ), XOR( "handwraps" ), XOR( "moto" ), XOR( "specialist" ) } );
		glove.SetCallback( callbacks::ForceFullUpdate );
		RegisterElement( &glove, 1 );

		glove_id.setup( XOR( "glove paintkit id" ), XOR( "skins_glove_id" ), 2 );
		glove_id.SetCallback( callbacks::ForceFullUpdate );
		RegisterElement( &glove_id, 1 );

	}
};

class MiscTab : public Tab
{
public:
	Dropdown      tab;

	// col1.
	MultiDropdown buy1;
	MultiDropdown buy2;
	MultiDropdown buy3;
	MultiDropdown notifications;
	Keybind       last_tick_defuse;
	Checkbox      fake_latency;
	Slider		  fake_latency_amt;
	Keybind       secondary_latency;
	Slider		  secondary_latency_amt;

	Checkbox disablebuybot;

	// col2.
	Checkbox print_network;
	Checkbox interpolation;
	MultiDropdown hitmarker;
	Slider hitmarker_time;
	Checkbox hitsound;
	Checkbox ragdoll_force;
	Checkbox killfeed;
	Checkbox ranks;
	MultiDropdown animations;
	Slider   aspect_ratio;
public:
	void init( )
	{
		SetTitle( XOR( "misc" ) );

		tab.setup( XOR( "" ), XOR( "misc_tab" ), { XOR( "general" ), XOR( "whitelist" ) }, false );
		RegisterElement( &tab );

		buy1.setup( XOR( "auto buy items" ), XOR( "auto_buy1" ),
			{
				XOR( "galilar" ),
				XOR( "famas" ),
				XOR( "ak47" ),
				XOR( "m4a1" ),
				XOR( "m4a1_silencer" ),
				XOR( "ssg08" ),
				XOR( "aug" ),
				XOR( "sg556" ),
				XOR( "awp" ),
				XOR( "scar20" ),
				XOR( "g3sg1" ),
				XOR( "nova" ),
				XOR( "xm1014" ),
				XOR( "mag7" ),
				XOR( "m249" ),
				XOR( "negev" ),
				XOR( "mac10" ),
				XOR( "mp9" ),
				XOR( "mp7" ),
				XOR( "ump45" ),
				XOR( "p90" ),
				XOR( "bizon" ),
			} );
		//buy1.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &buy1 );

		buy2.setup( "", XOR( "auto_buy2" ),
			{
				XOR( "glock" ),
				XOR( "hkp2000" ),
				XOR( "usp_silencer" ),
				XOR( "elite" ),
				XOR( "p250" ),
				XOR( "tec9" ),
				XOR( "fn57" ),
				XOR( "deagle" ),
			}, false );
		//buy2.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &buy2 );

		buy3.setup( "", XOR( "auto_buy3" ),
			{
				XOR( "vest" ),
				XOR( "vesthelm" ),
				XOR( "taser" ),
				XOR( "defuser" ),
				XOR( "heavyarmor" ),
				XOR( "molotov" ),
				XOR( "incgrenade" ),
				XOR( "decoy" ),
				XOR( "flashbang" ),
				XOR( "hegrenade" ),
				XOR( "smokegrenade" ),
			}, false );
		//buy3.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &buy3 );

		notifications.setup( XOR( "notifications" ), XOR( "notifications" ), { XOR( "matchmaking" ), XOR( "damage" ), XOR( "purchases" ), XOR( "bomb" ), XOR( "defuse" ), XOR( "shots" ), XOR( "misses" ) } );
		//notifications.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &notifications );

	//	last_tick_defuse.setup( XOR( "last tick defuse" ), XOR( "last_tick_defuse" ) );
	//	last_tick_defuse.AddShowCallback( callbacks::IsMisc );
	//	RegisterElement( &last_tick_defuse );

		fake_latency.setup( XOR( "fake latency" ), XOR( "fake_latency" ) );
	//	fake_latency.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &fake_latency );

		fake_latency_amt.setup( "", XOR( "fake_latency_amt" ), 0.f, 800.f, false, 0, 200.f, 10.f, XOR( L"ms" ) );
	//	fake_latency_amt.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &fake_latency_amt );

		secondary_latency.setup( XOR( "fake latency" ), XOR( "secondary_latency" ) );
		secondary_latency.SetToggleCallback( callbacks::ToggleFakeLatency );
	//	secondary_latency.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &secondary_latency );

	/*	secondary_latency_amt.setup( "", XOR( "secondary_latency_amt" ), 0.f, 800.f, false, 0, 200.f, 10.f, XOR( L"ms" ) );
		secondary_latency_amt.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &secondary_latency_amt );*/

		disablebuybot.setup( XOR( "disable buybot on pistol round" ), XOR( "disablebuybot" ) );
	//	disablebuybot.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &disablebuybot );

		// col2.
		/*print_network.setup( XOR( "show voicedata (debug)" ), XOR( "print_network" ) );
		print_network.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &print_network, 1 );*/

		hitmarker.setup( XOR( "hitmarker" ), XOR( "hitmarker" ), { XOR( "crosshair" ), XOR( "world" ) } );
	//	hitmarker.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &hitmarker, 1 );

		hitmarker_time.setup( XOR( "hitmarker time" ), XOR( "hitmarker_time" ), 1, 4, true, 0, 1, 1, L"s" );
	//	hitmarker_time.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &hitmarker_time, 1 );

		hitsound.setup( XOR( "hitsound" ), XOR( "hitsound" ) );
	//	hitsound.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &hitsound, 1 );

		ragdoll_force.setup( XOR( "ragdoll force" ), XOR( "ragdoll_force" ) );
	//	ragdoll_force.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &ragdoll_force, 1 );

	//	ranks.setup( XOR( "reveal matchmaking ranks" ), XOR( "ranks" ) );
	//	ranks.AddShowCallback( callbacks::IsMisc );
	//	RegisterElement( &ranks, 1 );

		killfeed.setup( XOR( "preserve killfeed" ), XOR( "killfeed" ) );
	//	killfeed.SetCallback( callbacks::ToggleKillfeed );
	//	killfeed.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &killfeed, 1 );

		aspect_ratio.setup(XOR("aspect ratio"), XOR("aspect_ratio"), 0.0f, 2, true, 1, 1.8f, 0.1f);
		RegisterElement(&aspect_ratio );

	//	interpolation.setup( XOR( "enemy interpolation" ), XOR( "interpolation" ) );
	//	interpolation.AddShowCallback( callbacks::IsMisc );
	//	RegisterElement( &interpolation, 1 );

		animations.setup( XOR( "local animations" ), XOR( "animations" ), { XOR( "static legs in air" ), XOR( "zero pitch on land" ), XOR( "remove desync jitter" ), XOR( "remove shot animation" ), XOR( "interpolation" ), XOR( "remove flick" ) } );
	//	animations.AddShowCallback( callbacks::IsMisc );
		RegisterElement( &animations, 1 );
	}
};

class Whitelist : public Tab
{
public:
	Edit   player_id;
	Button whitelist_id;
	Button remove_id;
	Button clear_ids;
public:

	void init()
	{
		SetTitle(XOR("player list"));

		player_id.setup(XOR("player id"), XOR("id"), 20);
		RegisterElement(&player_id);

		whitelist_id.setup(XOR("add id"));
		whitelist_id.SetCallback(callbacks::WhitelistAdd);
		RegisterElement(&whitelist_id);

		remove_id.setup(XOR("remove id"));
		remove_id.SetCallback(callbacks::WhitelistRemove);
		RegisterElement(&remove_id);

		clear_ids.setup(XOR("clear ids"));
		clear_ids.SetCallback(callbacks::WhitelistClear);
		RegisterElement(&clear_ids);
	}
};

class ConfigTab : public Tab
{
public:
	Colorpicker menu_color;

	Dropdown mode;
	Dropdown config;
	Keybind  key1;
	Keybind  key2;
	Keybind  key3;
	Keybind  key4;
	Keybind  key5;
	Keybind  key6;
	Button   save;
	Button   load;
	Button   resetconfig;
	Keybind  menu_key;
	Checkbox testing;
	Checkbox config_logs;

public:

	void init( )
	{
		SetTitle( XOR( "config" ) );

		menu_color.setup( XOR( "menu color" ), XOR( "menu_color" ), colors::burgundy, &g_gui.m_color );
		RegisterElement( &menu_color );

		menu_key.setup( XOR( "menu key" ), XOR( "menu_key" ), VK_INSERT );
		RegisterElement( &menu_key );

		testing.setup(XOR("testing mode"), XOR("testing"));
		RegisterElement(&testing);

		mode.setup( XOR( "safety mode" ), XOR( "mode" ), { XOR( "matchmaking" ), XOR( "no-spread" ) } );
		RegisterElement( &mode, 1 );

		config.setup( XOR( "configuration" ), XOR( "cfg" ), { XOR( "1" ), XOR( "2" ), XOR( "3" ), XOR( "4" ), XOR( "5" ), XOR( "6" ) } );
		config.RemoveFlags( ElementFlags::SAVE );
		RegisterElement( &config, 1 );

		key1.setup( XOR( "key 1" ), XOR( "cfg_key1" ) );
		key1.RemoveFlags( ElementFlags::SAVE );
		key1.SetCallback( callbacks::SaveHotkeys );
		key1.AddShowCallback( callbacks::IsConfig1 );
		key1.SetToggleCallback( callbacks::ConfigLoad1 );
		RegisterElement( &key1, 1 );

		key2.setup( XOR( "key 2" ), XOR( "cfg_key2" ) );
		key2.RemoveFlags( ElementFlags::SAVE );
		key2.SetCallback( callbacks::SaveHotkeys );
		key2.AddShowCallback( callbacks::IsConfig2 );
		key2.SetToggleCallback( callbacks::ConfigLoad2 );
		RegisterElement( &key2, 1 );

		key3.setup( XOR( "key 3" ), XOR( "cfg_key3" ) );
		key3.RemoveFlags( ElementFlags::SAVE );
		key3.SetCallback( callbacks::SaveHotkeys );
		key3.AddShowCallback( callbacks::IsConfig3 );
		key3.SetToggleCallback( callbacks::ConfigLoad3 );
		RegisterElement( &key3, 1 );

		key4.setup( XOR( "key 4" ), XOR( "cfg_key4" ) );
		key4.RemoveFlags( ElementFlags::SAVE );
		key4.SetCallback( callbacks::SaveHotkeys );
		key4.AddShowCallback( callbacks::IsConfig4 );
		key4.SetToggleCallback( callbacks::ConfigLoad4 );
		RegisterElement( &key4, 1 );

		key5.setup( XOR( "key 5" ), XOR( "cfg_key5" ) );
		key5.RemoveFlags( ElementFlags::SAVE );
		key5.SetCallback( callbacks::SaveHotkeys );
		key5.AddShowCallback( callbacks::IsConfig5 );
		key5.SetToggleCallback( callbacks::ConfigLoad5 );
		RegisterElement( &key5, 1 );

		key6.setup( XOR( "key 6" ), XOR( "cfg_key6" ) );
		key6.RemoveFlags( ElementFlags::SAVE );
		key6.SetCallback( callbacks::SaveHotkeys );
		key6.AddShowCallback( callbacks::IsConfig6 );
		key6.SetToggleCallback( callbacks::ConfigLoad6 );
		RegisterElement( &key6, 1 );

		save.setup( XOR( "save" ) );
		save.SetCallback( callbacks::ConfigSave );
		RegisterElement( &save, 1 );

		load.setup( XOR( "load" ) );
		load.SetCallback( callbacks::ConfigLoad );
		RegisterElement( &load, 1 );

		config_logs.setup(XOR("config logs"), XOR("config_logs"));
		RegisterElement(&config_logs, 1);
	}
};

class MainForm : public Form
{
public:
	// aimbot.
	AimbotTab    aimbot;
	AntiAimTab   antiaim;

	// visuals.
	PlayersTab	 players;
	VisualsTab	 visuals;

	// misc.
	MovementTab  movement;
	SkinsTab     skins;
	MiscTab	     misc;
	Whitelist    playerstab;
	ConfigTab	 config;

public:
	void init( )
	{
		SetPosition( 500, 350);
		SetSize( 630, 540 );

		// aim.
		RegisterTab( &aimbot );
		aimbot.init( );

		RegisterTab( &antiaim );
		antiaim.init( );

		// visuals.
		RegisterTab( &players );
		players.init( );

		RegisterTab( &visuals );
		visuals.init( );

		// misc.
		RegisterTab( &movement );
		movement.init( );

		RegisterTab( &skins );
		skins.init( );

		RegisterTab( &misc );
		misc.init( );

		RegisterTab(&playerstab);
		playerstab.init();

		RegisterTab( &config );
		config.init( );
	}
};

class Menu
{
public:
	MainForm main;

public:
	void init( )
	{
		main.init( );
		g_gui.RegisterForm( &main, VK_INSERT );
	}
};

extern Menu g_menu;