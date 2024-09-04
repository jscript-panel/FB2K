#pragma once

class DropSourceImpl : public ImplementCOMRefCounter<IDropSource>
{
public:
	COM_QI_SIMPLE(IDropSource)

	STDMETHODIMP GiveFeedback(DWORD dwEffect) final
	{
		m_effect = dwEffect;
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

	STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) final
	{
		if (fEscapePressed || WI_IsAnyFlagSet(grfKeyState, MK_RBUTTON | MK_MBUTTON))
			return DRAGDROP_S_CANCEL;

		if (!WI_IsFlagSet(grfKeyState, MK_LBUTTON))
		{
			if (m_effect == DROPEFFECT_NONE)
				return DRAGDROP_S_CANCEL;

			return DRAGDROP_S_DROP;
		}

		return S_OK;
	}

private:
	DWORD m_effect = DROPEFFECT_NONE;
};
