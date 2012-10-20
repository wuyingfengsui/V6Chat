function wnd_Cancel()
	local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("MessageTips")
	tipsHostWnd:Cancel()
		
	local app = XLGetObject("V6Chat3App")
	local unread, msgtype = app:GetUnreadMessage()
	
	if unread then
		app:PostWindowMessage("MainFrame", 0x0402, 0, 0)
	end
end

function choice1_OnMouseMove(self)
	self:SetCursorID ("IDC_HAND")
end

function choice1_OnMouseLeave(self)
    self:SetCursorID ("IDC_ARROW")
end

function choice1_btn_OnLButtonUp(self)
	local ownerWnd = self:GetOwner():GetBindHostWnd()
	local userData = ownerWnd:GetUserData()
	if userData.choiceFunc then
	    userData.choiceFunc(1, userData)
	end
	wnd_Cancel()
end

function choice2_OnMouseMove(self)
    self:SetCursorID ("IDC_HAND")
end

function choice2_OnMouseLeave(self)
    self:SetCursorID ("IDC_ARROW")
end

function choice2_btn_OnLButtonUp(self)
    local ownerWnd = self:GetOwner():GetBindHostWnd()
	local userData = ownerWnd:GetUserData()
	if userData.choiceFunc then
	    userData.choiceFunc(0, userData)
	end
	wnd_Cancel()
end

function close_btn_OnClick(self)            -- 点击关闭等同于choice2
    choice2_btn_OnLButtonUp(self)
end

function SetChildVisible(visible)
	local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("MessageTips")
	local tipsTree = tipsHostWnd:GetBindUIObjectTree()
				
	local message = tipsTree:GetUIObject("message")
	message:SetVisible(visible)
	
	local choice1 = tipsTree:GetUIObject("choice1")
	local choice2 = tipsTree:GetUIObject("choice2")
	if choice1 and choice2 then
	    choice1:SetVisible(visible)
		choice2:SetVisible(visible)
	end
end

function OnShowWindow(self, isShow)
    if isShow then
	    local data = self:GetUserData()
		local tipsTree = self:GetBindUIObjectTree()
		if data then
		    local message = tipsTree:GetUIObject("message")
		    message:SetText(data.message)
			local choice1 = tipsTree:GetUIObject("choice1")
			local choice2 = tipsTree:GetUIObject("choice2")
		    if choice1 and choice2 then
			    choice1:SetText(data.choice1)
				choice2:SetText(data.choice2)
			end
		end
		
		local function onAniFinish(self,oldState,newState)
		    if newState == 4 then
				SetChildVisible(true)
			end
	    end
		
	    local bkg = tipsTree:GetUIObject("bkg")
	    local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")	
	    local aniAlpha = aniFactory:CreateAnimation("AlphaChangeAnimation")
	    aniAlpha:BindRenderObj(bkg)
	    aniAlpha:SetTotalTime(250)
	    aniAlpha:SetKeyFrameAlpha(0,255)
		aniAlpha:AttachListener(true,onAniFinish)
	    tipsTree:AddAnimation(aniAlpha)
	    aniAlpha:Resume()
	end
end