#pragma once

class DropTargetImpl : public ImplementCOMRefCounter<IDropTarget>
{
public:
	DropTargetImpl(PanelBase* panel);

	COM_QI_SIMPLE(IDropTarget)

	STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) final;
	STDMETHODIMP DragLeave() final;
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) final;
	STDMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) final;

private:
	void invoke(CallbackID id, DWORD grfKeyState, const POINTL& pt);

	PanelBase* m_panel;
	bool m_accepted_type{};
	wil::com_ptr_t<DropAction> m_action;
};
