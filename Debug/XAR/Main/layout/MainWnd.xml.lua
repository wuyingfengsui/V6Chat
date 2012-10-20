--lua文件必须是UTF-8编码的(最好无BOM头)
function PopupTrayMenu(hostwnd,x,y)
	local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	if not objtreeManager:GetUIObjectTree("TrayMenu.Tree.Instance") then
	    local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")				
	    local menuTreeTemplate = templateMananger:GetTemplate("TrayMenu.Tree","ObjectTreeTemplate")
	    local menuTree = menuTreeTemplate:CreateInstance("TrayMenu.Tree.Instance")
	    local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")

	    local menuHostWndTemplate = templateMananger:GetTemplate("TrayMenu.Wnd", "HostWndTemplate")
	    local menuHostWnd = menuHostWndTemplate:CreateInstance("TrayMenu.Wnd.Instance")
	
	    menuHostWnd:BindUIObjectTree(menuTree)
	
	    menuHostWnd:TrackPopupMenu(hostwnd, x - 100, y - 30, x, y)
		
	    objtreeManager:DestroyTree("TrayMenu.Tree.Instance")
	    hostwndManager:RemoveHostWnd("TrayMenu.Wnd.Instance")
	end
end

function PopupMessageTipsWnd(msgtype, msg, uid)
    local msgData = {}
	local tipTemplate
	
	if msgtype == 6 or msgtype == 7 then                 --申请加好友（P2P和Server）
		tipTemplate = "Main.MessageWithChoice.Tips"
		
		msgData.message = msg  .. "想添加你为好友，是否同意？"
		msgData.choice1 = "同意"
		msgData.choice2 = "拒绝"
		
		local function ChoiceFunc(choice)
			local app = XLGetObject("V6Chat3App")
			app:AddFriendAck(choice)
			app:DeleteUnreadMessage()
		end
		msgData.choiceFunc = ChoiceFunc
	elseif msgtype == 10 then            --有新聊天信息提示
		tipTemplate = "Main.MessageWithChoice.Tips"
		
		local app = XLGetObject("V6Chat3App")
		local ui = app:GetUserInfoByID(uid)
		msgData.uid = uid
		msgData.message = ui.name .. "发来一条信息：\n" .. msg
		msgData.choice1 = "回复"
		msgData.choice2 = "忽略"
		
		local function ChoiceFunc(choice, data)
			local app = XLGetObject("V6Chat3App")
			if choice == 1 then
				app:PostWindowMessage("MainTab1", 0x0402, 0, data.uid)
			end
			
			app:DeleteUnreadMessage()
		end
		msgData.choiceFunc = ChoiceFunc
	end
	
	local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("MessageTips")
	if tipsHostWnd then
		hostWndManager:RemoveHostWnd("MessageTips")
		tipsHostWnd = nil
	end
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
	local tipsHostWndTemplate = templateMananger:GetTemplate("Main.Message.TipsWnd","HostWndTemplate")
	tipsHostWnd = tipsHostWndTemplate:CreateInstance("MessageTips")
	tipsHostWnd:SetTipTemplate(tipTemplate)
		
	local app = XLGetObject("V6Chat3App")
	local mx, my = app:GetWorkareaPos()
	tipsHostWnd:SetPosition(mx - 250, my - 150)
	
	tipsHostWnd:SetUserData(msgData)
	tipsHostWnd:Popup()
end


function close_btn_OnClick(self)
	local uiHelper = XLGetGlobal("V6ChatUIHelper")
	if uiHelper:V6MessageBox("是否退出程序？（选“否”则最小化）", 250, 130, 2) ~= 1 then
		local hwnd = self:GetOwner():GetBindHostWnd()
		hwnd:Show(0)
		return
	end
	
	local app = XLGetObject("V6Chat3App")
	app:Quit()
end


function min_btn_OnClick(self)
	local hwnd = self:GetOwner():GetBindHostWnd()
	hwnd:Show(0)
end


function my_head_OnClick(self)

end


function my_head_OnBind(self)
    local app = XLGetObject("V6Chat3App")
	self:SetHeadImage(app:GetMyHead())
end


function introduction_OnMouseMove(self)
	self:SetTextFontResID ("font.category.item.underline")
    self:SetCursorID ("IDC_HAND")
end


function introduction_OnMouseLeave(self)
	self:SetTextFontResID ("font.category.item")
end


function introduction_OnLButtonDown(self)
	self:SetVisible(false)
	local tree = self:GetOwner()
	local text = self:GetText()
	local edit_introduction = tree:GetUIObject("edit_introduction")
	if text == "点此设置签名" then
		edit_introduction:SetText("")
	else
		edit_introduction:SetText(text)
	end
	edit_introduction:SetVisible(true)
	edit_introduction:SetFocus(true)
end


function edit_introduction_OnFocusChange(self, isFocus, lastFocusObj)
	if not isFocus then
		self:SetVisible(false)
		local text = self:GetText()
		local tree = self:GetOwner()
		local introduction = tree:GetUIObject("introduction")
		if text == "" then
			introduction:SetText("点此设置签名")
		else
			introduction:SetText(text)
		end
		local app = XLGetObject("V6Chat3App")
		app:SetMyIntroduction(text)
		introduction:SetVisible(true)
	end
end


function tabs_OnBind(self)
	local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
	local xarManager = XLGetObject("Xunlei.UIEngine.XARManager")
	
	local Tab1HostWndTemplate = templateMananger:GetTemplate("Main.Tabs.FriendListWnd","HostWndTemplate")
	local tab1HostWnd = Tab1HostWndTemplate:CreateInstance("MainTab1")
	local tab1TreeTemplate = templateMananger:GetTemplate("Main.Tabs.FriendList","ObjectTreeTemplate")
	local tab1Tree = tab1TreeTemplate:CreateInstance("MainTab1Tree")
	tab1HostWnd:BindUIObjectTree(tab1Tree)
	tab1HostWnd:Create()
	
	local tab1 = objFactory:CreateUIObject("tab1","RealObject")
	tab1:SetObjPos2(8,50,280,300)
	tab1:SetWindow(tab1HostWnd:GetWndHandle())
	
	local tab1Icon = objFactory:CreateUIObject("tab1Icon","ImageObject")
	tab1Icon:SetResProvider(xarManager)
	tab1Icon:SetObjPos2(125,10,30,30)
	tab1Icon:SetResID("main.friendList.icon")
	--[[
	local Tab2HostWndTemplate = templateMananger:GetTemplate("Main.Tabs.GroupListWnd","HostWndTemplate")
	local tab2HostWnd = Tab2HostWndTemplate:CreateInstance("MainTab2")
	local tab2TreeTemplate = templateMananger:GetTemplate("Main.Tabs.GroupList","ObjectTreeTemplate")
	local tab2Tree = tab2TreeTemplate:CreateInstance("MainTab2Tree")
	tab2HostWnd:BindUIObjectTree(tab2Tree)
	tab2HostWnd:Create()
	
	local tab2 = objFactory:CreateUIObject("tab2","RealObject")
	tab2:SetObjPos2(8,50,280,300)
	tab2:SetWindow(tab2HostWnd:GetWndHandle())
	
	local Tab3HostWndTemplate = templateMananger:GetTemplate("Main.Tabs.SharedFilesListWnd","HostWndTemplate")
	local tab3HostWnd = Tab3HostWndTemplate:CreateInstance("MainTab3")
	local tab3TreeTemplate = templateMananger:GetTemplate("Main.Tabs.SharedFilesList","ObjectTreeTemplate")
	local tab3Tree = tab3TreeTemplate:CreateInstance("MainTab3Tree")
	tab3HostWnd:BindUIObjectTree(tab3Tree)
	tab3HostWnd:Create()
	
	local tab3 = objFactory:CreateUIObject("tab3","RealObject")
	tab3:SetObjPos2(8,50,280,300)
	tab3:SetWindow(tab3HostWnd:GetWndHandle())
	]]--
	
	local Tab4HostWndTemplate = templateMananger:GetTemplate("Main.Tabs.ConfigWnd","HostWndTemplate")
	local tab4HostWnd = Tab4HostWndTemplate:CreateInstance("MainTab4")
	local tab4TreeTemplate = templateMananger:GetTemplate("Main.Tabs.Config","ObjectTreeTemplate")
	local tab4Tree = tab4TreeTemplate:CreateInstance("MainTab4Tree")
	tab4HostWnd:BindUIObjectTree(tab4Tree)
	tab4HostWnd:Create()
	
	local tab4 = objFactory:CreateUIObject("tab4","RealObject")
	tab4:SetObjPos2(8,50,280,300)
	tab4:SetWindow(tab4HostWnd:GetWndHandle())
	
	local tab4Icon = objFactory:CreateUIObject("tab4Icon","ImageObject")
	tab4Icon:SetResProvider(xarManager)
	tab4Icon:SetObjPos2(125,10,30,30)
	tab4Icon:SetResID("main.config.icon")
	
	local root = tab1Tree:GetUIObject("tree"):GetRootItem()
	if #root.children ~= 0 then
		self:AddTab(tab1, tab1Icon, true)
		self:AddTab(tab4, tab4Icon, false)
	else
		self:AddTab(tab1, tab1Icon, false)
		self:AddTab(tab4, tab4Icon, true)
	end
	--self:AddTab(tab2, false)
	--self:AddTab(tab3, false)
end


function tree_OnBindHostWnd(tree, hostwnd, isBind)
    if isBind then
	    local app = XLGetObject("V6Chat3App")
	    local name_txt, sex_txt, age_txt, introduction_txt = app:GetMyInfo()
	
	    local name = tree:GetUIObject("name")
		name:SetText(name_txt)
		local introduction = tree:GetUIObject("introduction")
		if introduction_txt == "" then
			introduction:SetText("点此设置签名")
		else
			introduction:SetText(introduction_txt)
		end
	end
end


function main_wnd_OnCreate(hostwnd)
	local app = XLGetObject("V6Chat3App")
	app:AddTray(hostwnd:GetWndHandle())
	
	local function MessageFilter(self, msg, wparam, lparam)
		if msg == 0x0401 then               --托盘事件
		    if lparam == 0x0204 then           --右击事件
			    local app = XLGetObject("V6Chat3App")
	            local x, y = app:GetCursorPos()
				PopupTrayMenu(hostwnd, x, y)
			elseif lparam == 515 then          --双击事件
				hostwnd:Show(1)
				hostwnd:BringWindowToTop(1)
			end
		elseif msg == 0x0010 then                --退出消息
			local app = XLGetObject("V6Chat3App")
			app:Quit()
		elseif msg == 0x0402 then               --消息提示
			local app = XLGetObject("V6Chat3App")
	        local unread, msgtype, uid = app:GetUnreadMessage()
				
			PopupMessageTipsWnd(msgtype, unread, uid)
		elseif msg == 0x0403 then
		    local uiHelper = XLGetGlobal("V6ChatUIHelper")
			uiHelper:V6MessageBox("对方已收到消息，请等待对方确认。", 500, 130)
		end
	end
	
	local cookie, ret = hostwnd:AddInputFilter(nil, MessageFilter)
	main_wnd_cookie = cookie
	
	hostwnd:Center()
end

function main_wnd_OnDestroy(hostwnd)
    local app = XLGetObject("V6Chat3App")
	app:DeleteTray()
	
	hostwnd:RemoveInputFilter(main_wnd_cookie)
end