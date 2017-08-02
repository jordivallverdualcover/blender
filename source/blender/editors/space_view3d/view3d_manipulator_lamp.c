/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/editors/space_view3d/view3d_manipulator_lamp.c
 *  \ingroup spview3d
 */


#include "BLI_blenlib.h"
#include "BLI_math.h"
#include "BLI_utildefines.h"

#include "BKE_context.h"
#include "BKE_object.h"

#include "DNA_object_types.h"
#include "DNA_lamp_types.h"

#include "ED_screen.h"
#include "ED_manipulator_library.h"

#include "MEM_guardedalloc.h"

#include "RNA_access.h"

#include "WM_api.h"
#include "WM_types.h"

#include "view3d_intern.h"  /* own include */

/* -------------------------------------------------------------------- */

/** \name Spot Lamp Manipulators
 * \{ */

/* Spot Lamp */

static bool WIDGETGROUP_lamp_spot_poll(const bContext *C, wmManipulatorGroupType *UNUSED(wgt))
{
	Object *ob = CTX_data_active_object(C);

	if (ob && ob->type == OB_LAMP) {
		Lamp *la = ob->data;
		return (la->type == LA_SPOT);
	}
	return false;
}

static void WIDGETGROUP_lamp_spot_setup(const bContext *UNUSED(C), wmManipulatorGroup *mgroup)
{
	const float color[4] = {0.5f, 0.5f, 1.0f, 1.0f};
	const float color_hi[4] = {0.8f, 0.8f, 0.45f, 1.0f};

	wmManipulatorWrapper *wwrapper = MEM_mallocN(sizeof(wmManipulatorWrapper), __func__);

	wwrapper->manipulator = WM_manipulator_new("MANIPULATOR_WT_arrow_3d", mgroup, NULL);
	RNA_enum_set(wwrapper->manipulator->ptr, "draw_options",  ED_MANIPULATOR_ARROW_STYLE_INVERTED);

	mgroup->customdata = wwrapper;

	ED_manipulator_arrow3d_set_range_fac(wwrapper->manipulator, 4.0f);
	WM_manipulator_set_color(wwrapper->manipulator, color);
	WM_manipulator_set_color_highlight(wwrapper->manipulator, color_hi);
}

static void WIDGETGROUP_lamp_spot_refresh(const bContext *C, wmManipulatorGroup *mgroup)
{
	wmManipulatorWrapper *wwrapper = mgroup->customdata;
	Object *ob = CTX_data_active_object(C);
	Lamp *la = ob->data;
	float dir[3];

	negate_v3_v3(dir, ob->obmat[2]);

	WM_manipulator_set_matrix_rotation_from_z_axis(wwrapper->manipulator, dir);
	WM_manipulator_set_matrix_location(wwrapper->manipulator, ob->obmat[3]);

	/* need to set property here for undo. TODO would prefer to do this in _init */
	PointerRNA lamp_ptr;
	const char *propname = "spot_size";
	RNA_pointer_create(&la->id, &RNA_Lamp, la, &lamp_ptr);
	WM_manipulator_target_property_def_rna(wwrapper->manipulator, "offset", &lamp_ptr, propname, -1);
}

void VIEW3D_WGT_lamp_spot(wmManipulatorGroupType *wgt)
{
	wgt->name = "Spot Lamp Widgets";
	wgt->idname = "VIEW3D_WGT_lamp_spot";

	wgt->flag |= (WM_MANIPULATORGROUPTYPE_PERSISTENT |
	              WM_MANIPULATORGROUPTYPE_3D |
	              WM_MANIPULATORGROUPTYPE_DEPTH_3D);

	wgt->poll = WIDGETGROUP_lamp_spot_poll;
	wgt->setup = WIDGETGROUP_lamp_spot_setup;
	wgt->refresh = WIDGETGROUP_lamp_spot_refresh;
}

/** \} */

/* -------------------------------------------------------------------- */

/** \name Area Lamp Manipulators
 * \{ */

/* Area Lamp */

/* translate callbacks */
static void manipulator_area_lamp_prop_size_get(
        const wmManipulator *UNUSED(mpr), wmManipulatorProperty *mpr_prop,
        void *value_p)
{
	float *value = value_p;
	BLI_assert(mpr_prop->type->array_length == 2);
	UNUSED_VARS_NDEBUG(mpr_prop);
	Lamp *la = mpr_prop->custom_func.user_data;

	value[0] = la->area_size;
	value[1] = (la->area_shape == LA_AREA_RECT) ? la->area_sizey : la->area_size;
}

static void manipulator_area_lamp_prop_size_set(
        const wmManipulator *UNUSED(mpr), wmManipulatorProperty *mpr_prop,
        const void *value_p)
{
	const float *value = value_p;

	BLI_assert(mpr_prop->type->array_length == 2);
	UNUSED_VARS_NDEBUG(mpr_prop);
	Lamp *la = mpr_prop->custom_func.user_data;
	if (la->area_shape == LA_AREA_RECT) {
		la->area_size = value[0];
		la->area_sizey = value[1];
	}
	else {
		la->area_size = value[0];
	}
}

static void manipulator_area_lamp_prop_size_range(
        const wmManipulator *UNUSED(mpr), wmManipulatorProperty *UNUSED(mpr_prop),
        void *value_p)
{
	float *value = value_p;
	value[0] = 0.0f;
	value[1] =  FLT_MAX;
}

static bool WIDGETGROUP_lamp_area_poll(const bContext *C, wmManipulatorGroupType *UNUSED(wgt))
{
	Object *ob = CTX_data_active_object(C);

	if (ob && ob->type == OB_LAMP) {
		Lamp *la = ob->data;
		return (la->type == LA_AREA);
	}
	return false;
}

static void WIDGETGROUP_lamp_area_setup(const bContext *UNUSED(C), wmManipulatorGroup *mgroup)
{
	const float color[4] = {1.0f, 1.0f, 0.5f, 1.0f};
	const float color_hi[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	wmManipulatorWrapper *wwrapper = MEM_mallocN(sizeof(wmManipulatorWrapper), __func__);
	wwrapper->manipulator = WM_manipulator_new("MANIPULATOR_WT_cage_2d", mgroup, NULL);

	RNA_enum_set(wwrapper->manipulator->ptr, "transform",
	             ED_MANIPULATOR_RECT_TRANSFORM_FLAG_SCALE);

	const float dims[4] = {1.0f, 1.0f};
	RNA_float_set_array(wwrapper->manipulator->ptr, "dimensions", dims);

	mgroup->customdata = wwrapper;

	WM_manipulator_set_color(wwrapper->manipulator, color);
	WM_manipulator_set_color_highlight(wwrapper->manipulator, color_hi);
}

static void WIDGETGROUP_lamp_area_refresh(const bContext *C, wmManipulatorGroup *mgroup)
{
	wmManipulatorWrapper *wwrapper = mgroup->customdata;
	Object *ob = CTX_data_active_object(C);
	Lamp *la = ob->data;

	copy_m4_m4(wwrapper->manipulator->matrix_basis, ob->obmat);

	RNA_enum_set(wwrapper->manipulator->ptr, "transform",
	             ED_MANIPULATOR_RECT_TRANSFORM_FLAG_SCALE |
	             ((la->area_shape == LA_AREA_SQUARE) ? ED_MANIPULATOR_RECT_TRANSFORM_FLAG_SCALE_UNIFORM : 0));

	/* need to set property here for undo. TODO would prefer to do this in _init */
	WM_manipulator_target_property_def_func(
	        wwrapper->manipulator, "scale",
	        &(const struct wmManipulatorPropertyFnParams) {
	            .value_get_fn = manipulator_area_lamp_prop_size_get,
	            .value_set_fn = manipulator_area_lamp_prop_size_set,
	            .range_get_fn = manipulator_area_lamp_prop_size_range,
	            .user_data = la,
	        });
}

void VIEW3D_WGT_lamp_area(wmManipulatorGroupType *wgt)
{
	wgt->name = "Area Lamp Widgets";
	wgt->idname = "VIEW3D_WGT_lamp_area";

	wgt->flag |= (WM_MANIPULATORGROUPTYPE_PERSISTENT |
	              WM_MANIPULATORGROUPTYPE_3D |
	              WM_MANIPULATORGROUPTYPE_DEPTH_3D);

	wgt->poll = WIDGETGROUP_lamp_area_poll;
	wgt->setup = WIDGETGROUP_lamp_area_setup;
	wgt->refresh = WIDGETGROUP_lamp_area_refresh;
}

/** \} */
