#include "stdafx.hpp"
#include "DropTargetImpl.hpp"

#include "ProcessLocationsNotify.hpp"

DropTargetImpl::DropTargetImpl(PanelBase* panel) : m_panel(panel), m_action(new ImplementCOMRefCounter<DropAction>()) {}

STDMETHODIMP DropTargetImpl::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	RETURN_HR_IF(E_INVALIDARG, !pDataObj || !pdwEffect);

	m_action->Reset();
	m_accepted_type = playlist_incoming_item_filter::get()->process_dropped_files_check(pDataObj);

	if (m_accepted_type)
	{
		m_action->m_effect = *pdwEffect;
		invoke(CallbackID::on_drag_enter, grfKeyState, pt);
		*pdwEffect = m_action->m_effect;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP DropTargetImpl::DragLeave()
{
	m_panel->m_script_host->InvokeCallback(CallbackID::on_drag_leave);
	return S_OK;
}

STDMETHODIMP DropTargetImpl::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	RETURN_HR_IF(E_INVALIDARG, !pdwEffect);

	if (m_accepted_type)
	{
		m_action->m_effect = *pdwEffect;
		invoke(CallbackID::on_drag_over, grfKeyState, pt);
		*pdwEffect = m_action->m_effect;
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP DropTargetImpl::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	RETURN_HR_IF(E_INVALIDARG, !pDataObj || !pdwEffect);

	if (m_accepted_type)
	{
		m_action->m_effect = *pdwEffect;
		invoke(CallbackID::on_drag_drop, grfKeyState, pt);

		if (m_action->m_effect != DROPEFFECT_NONE)
		{
			const auto id = Plman::get_id(m_action->m_playlistIndex);
			if (id != Plman::invalid_id)
			{
				auto ptr = fb2k::service_new<ProcessLocationsNotify>(id, m_action->m_base, m_action->m_to_select);
				playlist_incoming_item_filter_v2::get()->process_dropped_files_async(pDataObj, playlist_incoming_item_filter_v2::op_flag_delay_ui, m_panel->GetWnd(), ptr);
				*pdwEffect = m_action->m_effect;
				return S_OK;
			}
		}
	}

	*pdwEffect = DROPEFFECT_NONE;
	return S_OK;
}

void DropTargetImpl::invoke(CallbackID id, DWORD grfKeyState, const POINTL& pt)
{
	CPoint point(pt.x, pt.y);
	m_panel->GetWnd().ScreenToClient(&point);
	m_panel->m_script_host->InvokeCallback(id, { m_action.get(), point.x, point.y, grfKeyState });
}
