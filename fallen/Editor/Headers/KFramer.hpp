// KFramer.hpp
// Guy Simmons, 19th September 1997.


#ifndef	_KFRAMER_HPP_
#define	_KFRAMER_HPP_

#include	"EditMod.hpp"


#define	CTRL_KF_LOAD_BUTTON			1
#define	CTRL_KF_FRAME_SLIDER		2


class	KeyFrameEditor2	:	public	EditorModule
{
	private:
		SLONG			AnimAngleX,
						AnimAngleY,
						AnimAngleZ,
						AnimOffsetX,
						AnimOffsetY,
						AnimOffsetZ,
						AnimScale,
						AnimCount,
						AnimTween;
		ControlSet		KeyFramesControls;
		EdRect			KeyFramesRect;

	public:
						~KeyFrameEditor2();
		void			SetupModule(void);
		void			DrawContent(void);
		void			HandleContentClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleControlClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleModule(void);
		void			HandleKeyFramesControl(ULONG control_id);
		void			DrawKeyFrames(void);
		void			DrawKeyFrame(UWORD multi_object,EdRect *bounds_rect,struct KeyFrame *the_frame,struct Matrix33 *r_matrix);
};

#endif

