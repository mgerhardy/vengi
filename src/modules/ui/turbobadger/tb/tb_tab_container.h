/**
 * @file
 */

#pragma once

#include "tb_widgets_common.h"

namespace tb {

/** TBTabLayout is a TBLayout used in TBTabContainer to apply
	some default properties on any TBButton added to it. */
class TBTabLayout : public TBLayout
{
public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(TBTabLayout, TBLayout);

	virtual void OnChildAdded(TBWidget *child) override;
	virtual PreferredSize OnCalculatePreferredContentSize(const SizeConstraints &constraints) override;
};

/** TBTabContainer - A container with tabs for multiple pages. */
class TBTabContainer : public TBWidget
{
public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(TBTabContainer, TBWidget);

	TBTabContainer();
	~TBTabContainer();

	/** Set along which axis the content should layouted.
		Use SetAlignment instead for more choice! Also, calling
		SetAxis directly does not update the current alignment. */
	virtual void SetAxis(AXIS axis) override;
	virtual AXIS GetAxis() const override { return m_root_layout.GetAxis(); }

	/** Set alignment of the tabs. */
	void SetAlignment(TB_ALIGN align);
	TB_ALIGN GetAlignment() const { return m_align; }

	/** Set which page should be selected and visible. */
	virtual void SetValue(int value) override;
	virtual int GetValue() const override { return m_current_page; }

	/** Set which page should be selected and visible. */
	void SetCurrentPage(int index) { SetValue(index); }
	int GetCurrentPage() { return GetValue(); }
	int GetNumPages();

	/** Return the widget that is the current page, or nullptr if none is active. */
	TBWidget *GetCurrentPageWidget() const;

	virtual void OnInflate(const INFLATE_INFO &info) override;
	virtual bool OnEvent(const TBWidgetEvent &ev) override;
	virtual void OnProcess() override;

	virtual TBWidget *GetContentRoot() override { return &m_content_root; }
	TBLayout *GetTabLayout() { return &m_tab_layout; }
protected:
	TBLayout m_root_layout;
	TBTabLayout m_tab_layout;
	TBWidget m_content_root;
	bool m_need_page_update;
	int m_current_page;
	TB_ALIGN m_align;
};

}
