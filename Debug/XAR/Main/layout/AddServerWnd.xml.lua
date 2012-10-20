function OnCreate(self)
    self:Center()
end

function OnShowWindow(self, isShow)
    if isShow then
	    local WndTree = self:GetBindUIObjectTree()
		
		local data = self:GetUserData()
		if data then
		
		if data.server_name then
			local server_name = WndTree:GetUIObject("server_name")
			server_name:SetText(data.server_name)
		end
		if data.domain then
			local admain = WndTree:GetUIObject("admain")
			admain:SetText(data.domain)
		end
		if data.account then
			local account = WndTree:GetUIObject("account")
			account:SetText(data.account)
		end
		if data.password then
			local password = WndTree:GetUIObject("password")
			password:SetText(data.password)
		end
		if data.introduction then
			local introduction = WndTree:GetUIObject("introduction")
			introduction:SetText(data.introduction)
		end
		
		end
		
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
		local server_name = tree:GetUIObject("server_name")
		local server_edit = server_name:GetControlObject("edit")
		local admain = tree:GetUIObject("admain")
		local admain_edit = admain:GetControlObject("edit")
		local account = tree:GetUIObject("account")
		local account_edit = account:GetControlObject("edit")
		local introduction = tree:GetUIObject("introduction")
		local introduction_edit = introduction:GetControlObject("edit")
		
	    if server_edit:GetText() == "" then
			server_edit:SetText("随便起个容易识别的名字吧")
		end
		
		if admain_edit:GetText() == "" then
			admain_edit:SetText("服务（网站）的域名，如talk.renren.com")
		end
		
		if account_edit:GetText() == "" then
			account_edit:SetText("在此服务（网站）上的账号名")
		end
		
		if introduction_edit:GetText() == "" then
			introduction_edit:SetText("如果没有什么要介绍的话直接保存即可")
		end
	end
end

function server_name_OnFocusChange(self, fun, focus)
	local server_edit = self:GetControlObject("edit")
	local text = server_edit:GetText()
	
	if not focus then
		if text == "" then
			server_edit:SetText("随便起个容易识别的名字吧")
		end
	else
		if text == "随便起个容易识别的名字吧" then
			server_edit:SetSelAll()
			server_edit:Clear()
		end
	end
end

function admain_OnFocusChange(self, fun, focus)
	local edit = self:GetControlObject("edit")
	local text = edit:GetText()
	
	if not focus then
		if text == "" then
			edit:SetText("服务（网站）的域名，如talk.renren.com")
		end
	else
		if text == "服务（网站）的域名，如talk.renren.com" then
			edit:SetSelAll()
			edit:Clear()
		end
	end
end

function account_OnFocusChange(self, fun, focus)
	local edit = self:GetControlObject("edit")
	local text = edit:GetText()
	
	if not focus then
		if text == "" then
			edit:SetText("在此服务（网站）上的账号名")
		end
	else
		if text == "在此服务（网站）上的账号名" then
			edit:SetSelAll()
			edit:Clear()
		end
	end
end

function introduction_OnFocusChange(self, fun, focus)
	local edit = self:GetControlObject("edit")
	local text = edit:GetText()
	
	if not focus then
		if text == "" then
			edit:SetText("如果没有什么要介绍的话直接保存即可")
		end
	else
		if text == "如果没有什么要介绍的话直接保存即可" then
			edit:SetSelAll()
			edit:Clear()
		end
	end
end

function OK_btn_OnClick(self)
    local owner = self:GetOwner()
	local wnd = owner:GetBindHostWnd()
	
	local server_name = owner:GetUIObject("server_name")
	local server_edit = server_name:GetControlObject("edit"):GetText()
	local admain = owner:GetUIObject("admain")
	local admain_edit = admain:GetControlObject("edit"):GetText()
	local account = owner:GetUIObject("account")
	local account_edit = account:GetControlObject("edit"):GetText()
	local password = owner:GetUIObject("password")
	local password_edit = password:GetControlObject("edit"):GetText()
	local introduction = owner:GetUIObject("introduction")
	local introduction_edit = introduction:GetControlObject("edit"):GetText()
		
	if server_edit == "随便起个容易识别的名字吧" then
	    local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("名称为空！", 250, 130)
		return
	end
	if admain_edit == "服务（网站）的域名，如talk.renren.com" then
	    local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("域名为空！", 250, 130)
		return
	end
	if account_edit == "在此服务（网站）上的账号名" then
	    local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("账号为空！", 250, 130)
		return
	end
	if password_edit == "" then
	    local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("密码为空！", 250, 130)
		return
	end
	if introduction_edit == "如果没有什么要介绍的话直接保存即可" then
	    introduction_edit = ""
	end
	
	local data = {server = server_edit, admain = admain_edit, account = account_edit, password = password_edit, introduction = introduction_edit}
	wnd:SetUserData(data)
	
	wnd:EndDialog(0)
end

function Cancel_btn_OnClick(self)
	local owner = self:GetOwner()
	local wnd = owner:GetBindHostWnd()
	wnd:EndDialog(1)
end