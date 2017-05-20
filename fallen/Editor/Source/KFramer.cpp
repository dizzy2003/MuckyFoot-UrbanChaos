// KFramer.cpp
// Guy Simmons, 19th September 1997.


#include	"Editor.hpp"
#include	"c:\fallen\headers\memory.h"

#define	CONTROLS_HEIGHT			400
#define	CONTROLS_WIDTH			200

#define	KEY_FRAME_COUNT			12
#define	KEY_FRAME_IMAGE_SIZE	48

#include	"KFDef2.c"


extern SLONG					key_frame_count;
extern struct KeyFrameChunk 	test_chunk2;

extern void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
extern void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);

// Used by fudgy centering bit.
extern SLONG		x_centre,
					y_centre,
					z_centre;

static KeyFrameEditor2		*the_editor;

void	test_draw_all_get_sizes(SWORD multi_prim,struct KeyFrame *the_frame,SLONG x,SLONG y,SLONG z,SLONG tween,struct Matrix33 *rot_mat,SLONG *width,SLONG *height,SLONG *mid_x,SLONG *mid_y);
void	update_key_frames(void);

//---------------------------------------------------------------

KeyFrameEditor2::~KeyFrameEditor2()
{
	
}

//---------------------------------------------------------------

void	KeyFrameEditor2::SetupModule(void)
{
	KeyFrameEdDefaults	the_defaults;


	the_editor			=	this;
	the_defaults.Left	=	0;
	the_defaults.Top	=	0;
	the_defaults.Width	=	CONTROLS_WIDTH+(KEY_FRAME_COUNT*KEY_FRAME_IMAGE_SIZE)+17;
	the_defaults.Height	=	400;
	SetupWindow	(
					"Key Frame Editor",
					(HAS_TITLE|HAS_CONTROLS),
					the_defaults.Left,
					the_defaults.Top,
					the_defaults.Width,
					the_defaults.Height
				);
	SetContentColour(CONTENT_COL);
	SetControlsHeight(CONTROLS_HEIGHT);
	SetControlsWidth(CONTROLS_WIDTH);

	AnimAngleX		=	0;
	AnimAngleY		=	0;
	AnimAngleZ		=	0;
	AnimOffsetX		=	0;
	AnimOffsetY		=	0;
	AnimOffsetZ		=	0;
	AnimScale		=	296;
	AnimTween		=	0;

	KeyFramesControls.InitControlSet(kframe_ctrls_def);
	((CHSlider*)KeyFramesControls.GetControlPtr(CTRL_KF_FRAME_SLIDER))->SetUpdateFunction(update_key_frames);
	KeyFramesRect.SetRect(2,KEY_FRAME_IMAGE_SIZE-20,(KEY_FRAME_COUNT*KEY_FRAME_IMAGE_SIZE)+2,KEY_FRAME_IMAGE_SIZE+2);
}

//---------------------------------------------------------------

void	KeyFrameEditor2::DrawContent(void)
{
	EdRect		temp_rect;


	SetContentDrawArea();
	ClearContent();

	// Draw key frame controls.
	temp_rect.SetRect	(
							ContentLeft(),
							ContentBottom()-(KEY_FRAME_IMAGE_SIZE<<1)+1,
							ContentWidth(),
							KEY_FRAME_IMAGE_SIZE<<1
						);
	KeyFramesControls.ControlSetBounds(&temp_rect);
	KeyFramesControls.DrawControlSet();
	KeyFramesControls.HiliteControlDrawArea(HILITE_COL,LOLITE_COL);

	// Draw key frames.
	DrawKeyFrames();
}

//---------------------------------------------------------------

void	KeyFrameEditor2::HandleContentClick(UBYTE flags,MFPoint *clicked_point)
{
	ULONG		cleanup,
				update;
	SLONG		c0,
				first_frame,
				selected_frame,
				x_diff,
				y_diff;
	EdRect		frame_rect,
				last_rect,
				temp_rect;
	MFPoint		local_point;


	HandleKeyFramesControl(KeyFramesControls.HandleControlSetClick(flags,clicked_point));

	local_point	=	*clicked_point;
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(KeyFramesRect.PointInRect(&local_point))
			{
				// Find out which frame hass been selected.
				selected_frame	=	-1;
				first_frame		=	((CHSlider*)KeyFramesControls.GetControlPtr(CTRL_KF_FRAME_SLIDER))->GetCurrentValue();

				for(c0=0;c0<KEY_FRAME_COUNT&&key_frame_count;c0++)
				{
					KeyFramesControls.SetControlDrawArea();
					frame_rect.SetRect	(
											3+(c0*KEY_FRAME_IMAGE_SIZE),
											(KEY_FRAME_IMAGE_SIZE)-19,
											KEY_FRAME_IMAGE_SIZE,KEY_FRAME_IMAGE_SIZE
										);
					if(frame_rect.PointInRect(&local_point))
					{
						selected_frame	=	first_frame+c0;
						break;
					}
				}

				// Allow selected frame to be dragged around.
				if(selected_frame>=0)
				{
					SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);

					temp_rect.SetRect	(
											frame_rect.GetLeft(),
											frame_rect.GetTop(),
											frame_rect.GetWidth(),
											frame_rect.GetHeight()
										);
					x_diff	=	clicked_point->X-temp_rect.GetLeft();
					y_diff	=	clicked_point->Y-temp_rect.GetTop();
					last_rect.SetRect(0,0,0,0);
					cleanup	=	0;
					update	=	0;
					while(SHELL_ACTIVE && LeftButton)
					{
						temp_rect.SetRect(MouseX-x_diff,MouseY-y_diff,temp_rect.GetWidth(),temp_rect.GetHeight());
						if(temp_rect.GetLeft()<0)
							temp_rect.MoveRect(0,temp_rect.GetTop());
						if(temp_rect.GetTop()<0)
							temp_rect.MoveRect(temp_rect.GetLeft(),0);
						if(temp_rect.GetRight()>=WorkScreenWidth)
							temp_rect.MoveRect(WorkScreenWidth-temp_rect.GetWidth(),temp_rect.GetTop());
						if(temp_rect.GetBottom()>=WorkScreenHeight)
							temp_rect.MoveRect(temp_rect.GetLeft(),WorkScreenHeight-temp_rect.GetHeight());

						if(MouseMoved)
						{
							MouseMoved	=	0;

							// Check to see if the frame is above anything relevent here.

							if(LockWorkScreen())
							{
								temp_rect.OutlineInvertedRect();
								UnlockWorkScreen();
							}
							ShowWorkScreen(0);
							if(LockWorkScreen())
							{
								temp_rect.OutlineInvertedRect();
								UnlockWorkScreen();
							}
						}
					}
					RequestUpdate();
				}
			}
			break;
		case	RIGHT_CLICK:
			break;
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor2::HandleControlClick(UBYTE flags,MFPoint *clicked_point)
{
}

//---------------------------------------------------------------

void	KeyFrameEditor2::HandleModule(void)
{
	UBYTE			update		=	0;
	MFPoint			mouse_point;
	static UBYTE	cleanup_content	=	0;
	static MFPoint	last_point;


	if(LastKey)
	{
		// Handle hot keys for controls here.
	}


	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;
	KeyFramesControls.HandleControlSet(&mouse_point);

	if(PointInContent(&mouse_point))
	{
		if(KeyFramesControls.PointInControlSet(&mouse_point))
		{
			if(MouseMoved)
			{
				MouseMoved		=	0;
				update			=	1;
				cleanup_content	=	1;
			}
		}
		else if(cleanup_content)
		{
			update			=	1;
			cleanup_content	=	0;
		}
	}
	else if(cleanup_content)
	{
		update			=	1;
		cleanup_content	=	0;
	}



	if(update)
	{
		if(LockWorkScreen())
		{
			DrawContent();
			DrawGrowBox();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
		if(LockWorkScreen())
		{
			DrawControls();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor2::HandleKeyFramesControl(ULONG control_id)
{
	SLONG			c0;
	FileRequester	*fr;


	switch(control_id)
	{
		case	0:
			break;
		case	CTRL_KF_LOAD_BUTTON:
			fr	=	new FileRequester("DATA\\","*.VUE","Load a VUE file",".VUE");
			fr->Draw();
			strcpy(test_chunk2.VUEName,fr->Path);
			strcat(test_chunk2.VUEName,fr->FileName);
			strcpy(test_chunk2.ASCName,test_chunk2.VUEName);
			strcpy(test_chunk2.ANMName,test_chunk2.VUEName);
			c0=0;
			while(test_chunk2.ASCName[c0]!='.' && test_chunk2.ASCName[c0]!=0)c0++;
			if(test_chunk2.ASCName[c0]=='.')
			{
				test_chunk2.ASCName[c0+1]	=	'A';
				test_chunk2.ASCName[c0+2]	=	'S';
				test_chunk2.ASCName[c0+3]	=	'C';
				test_chunk2.ASCName[c0+4]	=	0;

				test_chunk2.ANMName[c0+1]	=	'A';
				test_chunk2.ANMName[c0+2]	=	'N';
				test_chunk2.ANMName[c0+3]	=	'M';
				test_chunk2.ANMName[c0+4]	=	0;
			}
			delete	fr;
			RequestUpdate();

			if(read_multi_asc(test_chunk2.ASCName,0))
			{
				test_chunk2.MultiObject		=	next_prim_multi_object-1;
				test_chunk2.ElementCount	=	prim_multi_objects[test_chunk2.MultiObject].EndObject-prim_multi_objects[test_chunk2.MultiObject].StartObject;

				// Fudgy bit for centering.
				{
SLONG				c1,
					sp,ep;
struct PrimObject	*p_obj;

					
					for(c0=prim_multi_objects[test_chunk2.MultiObject].StartObject;c0<=prim_multi_objects[test_chunk2.MultiObject].EndObject;c0++)
					{
						p_obj   =	&prim_objects[c0];
						sp		=	p_obj->StartPoint;
						ep		=	p_obj->EndPoint;

						for(c1=sp;c1<ep;c1++)
						{
							prim_points[c1].X	-=	x_centre;
							prim_points[c1].Y	-=	y_centre;
							prim_points[c1].Z	-=	z_centre;
						}
					}
				}				
				load_multi_vue(&test_chunk2);
				((CHSlider*)KeyFramesControls.GetControlPtr(CTRL_KF_FRAME_SLIDER))->SetValueRange(0,key_frame_count-(KEY_FRAME_COUNT-1));
/*
				LoadAllAnims(&test_chunk2);
				LoadChunkTextureInfo(&test_chunk2);
*/
			}
			break;
		case	CTRL_KF_FRAME_SLIDER:
			if(LockWorkScreen())
			{
				DrawKeyFrames();
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
			break;
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor2::DrawKeyFrames(void)
{
	SLONG				c0,
						first_frame;
	EdRect				draw_rect,
						frame_rect;
	MFPoint				mouse_point;
	struct Matrix33		r_matrix;


	if(test_chunk2.MultiObject)
	{
		KeyFramesRect.FillRect(ACTIVE_COL);
		rotate_obj((SWORD)AnimAngleX,(SWORD)AnimAngleY,0,&r_matrix);
		first_frame	=	((CHSlider*)KeyFramesControls.GetControlPtr(CTRL_KF_FRAME_SLIDER))->GetCurrentValue();

		for(c0=0;c0<KEY_FRAME_COUNT;c0++)
		{
			KeyFramesControls.SetControlDrawArea();
			frame_rect.SetRect	(
									3+(c0*KEY_FRAME_IMAGE_SIZE),
									(KEY_FRAME_IMAGE_SIZE)-19,
									KEY_FRAME_IMAGE_SIZE,KEY_FRAME_IMAGE_SIZE
								);
 			mouse_point.X	=	MouseX;
			mouse_point.Y	=	MouseY;
			KeyFramesControls.GlobalToLocal(&mouse_point);
			if(frame_rect.PointInRect(&mouse_point))
			{
				frame_rect.FillRect(HILITE_COL);
			}

			draw_rect	=	frame_rect;
			draw_rect.OffsetRect(KeyFramesControls.ControlGetBounds()->GetLeft(),KeyFramesControls.ControlGetBounds()->GetTop());
			DrawKeyFrame(test_chunk2.MultiObject,&draw_rect,&test_chunk2.KeyFrames[first_frame+c0],&r_matrix);
		}
	}
	KeyFramesControls.SetControlDrawArea();
	KeyFramesRect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

void	KeyFrameEditor2::DrawKeyFrame(UWORD multi_object,EdRect *bounds_rect,struct KeyFrame *the_frame,struct Matrix33 *r_matrix)
{
	SLONG					c0,c1,c2,
							scale,
							scale_y,
							temp_scale,
							temp_x,
							temp_y,
							temp_z,
							temp_vw2,
							temp_vh2,
							width,height,
							*flags;
	EdRect					outline_rect;
	struct KeyFrameElement	*the_element;
	struct SVector			*rotate_vectors;
	SLONG					mid_x=0,mid_y=0;


	// Stop the compiler moaning.
	multi_object	=	multi_object;

	if(!test_chunk2.MultiObject)
		return;

	c1	=	0;
	flags			=	(SLONG*)MemAlloc(sizeof(SLONG)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");
	rotate_vectors	=	(struct SVector*)MemAlloc(sizeof(struct SVector)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");


	temp_scale		=	engine.Scale;
	temp_x			=	engine.X;
	temp_y			=	engine.Y;
	temp_z			=	engine.Z;
	temp_vw2		=	engine.VW2;
	temp_vh2		=	engine.VH2;

	engine.X		=	0;
	engine.Y		=	0;
	engine.Z		=	0;
	engine.Scale	=	5000;
	engine.ShowDebug=	0;
	engine.BucketSize=	MAX_BUCKETS>>4;

	SetWorkWindowBounds	(
							bounds_rect->GetLeft(),
							bounds_rect->GetTop(),
							bounds_rect->GetWidth(),
							bounds_rect->GetHeight()
						);
	outline_rect.SetRect(
							0,0,
							bounds_rect->GetWidth(),bounds_rect->GetHeight()
						);

	set_camera();
//	set_camera_to_base();
	

//md	LogText(" width %d height %d \n",width,height);
	test_draw_all_get_sizes(test_chunk2.MultiObject,the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);
	if(width>0 && height>0)
	{


		scale	=	(bounds_rect->GetWidth()<<16)/width;
		scale_y	=	(bounds_rect->GetHeight()<<16)/height;
		if(scale_y<scale)
			scale	=	scale_y;

//		scale			=	(scale*900)>>8;
		engine.Scale	=	(5000*scale)>>16;

		//calc new mids with this scale
		test_draw_all_get_sizes(test_chunk2.MultiObject,the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);

		mid_x-=bounds_rect->GetWidth()>>1;
		mid_y-=bounds_rect->GetHeight()>>1;
		engine.VW2-=mid_x;
		engine.VH2-=mid_y;

	}
//	LogText(" drawkeyframe scale %d \n",engine.Scale);

	for(c2=0,c0=prim_multi_objects[test_chunk2.MultiObject].StartObject;c0<prim_multi_objects[test_chunk2.MultiObject].EndObject;c0++)
	{
		the_element			=	&the_frame->FirstElement[c2++];
   		test_draw((SWORD)c0,0,0,0,0,the_element,the_element,r_matrix, NULL, NULL, NULL, NULL, NULL);
	}
	render_view(0);
	outline_rect.OutlineRect(0);

	engine.X		=	temp_x;
	engine.Y		=	temp_y;
	engine.Z		=	temp_z;
	engine.Scale	=	temp_scale;
	engine.ShowDebug=	1;
	engine.BucketSize=	MAX_BUCKETS;
	engine.VW2	=	temp_vw2;
	engine.VH2	=	temp_vh2;

	MemFree(rotate_vectors);
	MemFree(flags);
}

//---------------------------------------------------------------

void	update_key_frames(void)
{
	the_editor->DrawKeyFrames();
}

//---------------------------------------------------------------
