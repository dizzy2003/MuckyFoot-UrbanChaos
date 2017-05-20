//
// A simple editor for putting HM stuff around prims.
//

class HmTab : public ModeTab
{
	private:

		void draw_prim(UWORD prim);
		void draw_grid(UWORD prim);
		void draw_cog (UWORD prim);

	public:

		//
		// The bare minimum...
		//

		HmTab(EditorModule *parent);
	   ~HmTab();

	    void	DrawTabContent          (void);
		void	HandleTab               (MFPoint *current_point);
		UWORD	HandleTabClick          (UBYTE flags, MFPoint *clicked_point);
		void	HandleControl           (UWORD control_id);
		void	DrawModuleContent       (SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG	HandleModuleContentClick(MFPoint *clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h);
		UBYTE   RedrawModuleContent;

		//
		// Extra stuff...
		//
};
