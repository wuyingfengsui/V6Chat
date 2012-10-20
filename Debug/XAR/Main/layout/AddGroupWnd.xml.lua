function OnShowWindow(self, isShow)
    if isShow then
	    local WndTree = self:GetBindUIObjectTree()
	    local bkg = WndTree:GetUIObject("small_map")
	
	    local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")	
	    local aniAlpha = aniFactory:CreateAnimation("AlphaChangeAnimation")
	    aniAlpha:BindRenderObj(bkg)
	    aniAlpha:SetTotalTime(250)
	    aniAlpha:SetKeyFrameAlpha(0,255)
	    WndTree:AddAnimation(aniAlpha)
	    aniAlpha:Resume()
	end
end

function OnFocusChange(hwnd, focus)
    if focus then
	    local tree = hwnd:GetBindUIObjectTree()
		local group_name = tree:GetUIObject("group_name")
		local edit = group_name:GetControlObject("edit")
	    edit:SetFocus(true)
	end
end

function OK_btn_OnClick(self)
    local owner = self:GetOwner()
	local wnd = owner:GetBindHostWnd()
	local group_name = owner:GetUIObject("group_name")
	local name = group_name:GetText()
	if name and name ~= "" then
	    wnd:SetUserData(group_name:GetText())
	end
	wnd:EndDialog(0)
end

function Cancel_btn_OnClick(self)
	local owner = self:GetOwner()
	local wnd = owner:GetBindHostWnd()
	wnd:EndDialog(0)
end