function tree_OnHover(self, fun, item, y)
	local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("InfoTips")
	if not tipsHostWnd then
		local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
		local tipsHostWndTemplate = templateMananger:GetTemplate("Main.Info.TipsWnd","HostWndTemplate")
		tipsHostWnd = tipsHostWndTemplate:CreateInstance("InfoTips")
	end
	tipsHostWnd:SetTipTemplate("Main.GroupInfo.Tips")
	
	local friendsListHostWnd = hostWndManager:GetHostWnd("MainTab2")
	local wnd_left,wnd_top,wnd_right,wnd_bottom = friendsListHostWnd:GetWindowRect()
	if wnd_left>278 then
		tipsHostWnd:SetPositionByWindow(-278,y,friendsListHostWnd:GetWndHandle())
	else
		tipsHostWnd:SetPositionByWindow(wnd_right - wnd_left + 10,y,friendsListHostWnd:GetWndHandle())
	end
		
	local userData = {}
	userData.name = item.txt
	userData.owner = "test"
	userData.introduction = "西安电子科技大学，西安电子科技大学，西安电子科技大学"
	tipsHostWnd:SetUserData(userData)
		
	tipsHostWnd:Popup()
end

function tree_OnLeave(self, fun, item)
    local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("InfoTips")
	if tipsHostWnd then
	    tipsHostWnd:Destroy()
	end
end

function tree_OnSelected(self, fun, item)
	XLMessageBox(item.id)
end

function tree_OnPopupGroupMenu(self, fun, item)
    local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	if not objtreeManager:GetUIObjectTree("GroupMenu.Tree.Instance") then
	    local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")				
	    local menuTreeTemplate = templateMananger:GetTemplate("GroupMenu.Tree","ObjectTreeTemplate")
	    local menuTree = menuTreeTemplate:CreateInstance("GroupMenu.Tree.Instance")
	    local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")

	    local menuHostWndTemplate = templateMananger:GetTemplate("GroupMenu.Wnd", "HostWndTemplate")
	    local menuHostWnd = menuHostWndTemplate:CreateInstance("GroupMenu.Wnd.Instance")
	
	    menuHostWnd:BindUIObjectTree(menuTree)
	
	    local userData = {}
		userData.tree = self
		userData.item = item
		userData.groups = GroupsGroup
	    menuHostWnd:SetUserData(userData)
		
	    local app = XLGetObject("V6Chat3App")
	    local x, y = app:GetCursorPos()
	    menuHostWnd:TrackPopupMenu(hostwnd, x, y, x + 400, y + 400)
		
	    objtreeManager:DestroyTree("GroupMenu.Tree.Instance")
	    hostwndManager:RemoveHostWnd("GroupMenu.Wnd.Instance")
	end
end

function tree_OnPopupItemMenu(self, fun, item)
	local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	if not objtreeManager:GetUIObjectTree("GroupsListMenu.Tree.Instance") then
	    local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")				
	    local menuTreeTemplate = templateMananger:GetTemplate("GroupsListMenu.Tree","ObjectTreeTemplate")
	    local menuTree = menuTreeTemplate:CreateInstance("GroupsListMenu.Tree.Instance")
	    local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")

	    local menuHostWndTemplate = templateMananger:GetTemplate("GroupsListMenu.Wnd", "HostWndTemplate")
	    local menuHostWnd = menuHostWndTemplate:CreateInstance("GroupsListMenu.Wnd.Instance")
	
	    menuHostWnd:BindUIObjectTree(menuTree)
	
	    local userData = {}
		userData.tree = self
		userData.item = item
		userData.groups = GroupsGroup
	    menuHostWnd:SetUserData(userData)
		
	    local app = XLGetObject("V6Chat3App")
	    local x, y = app:GetCursorPos()
	    menuHostWnd:TrackPopupMenu(hostwnd, x, y, x + 400, y + 400)
		
	    objtreeManager:DestroyTree("GroupsListMenu.Tree.Instance")
	    hostwndManager:RemoveHostWnd("GroupsListMenu.Wnd.Instance")
	end
end

function tree_OnInitControl(self)
    local root = self:GetRootItem()
	GroupsGroup = {}
	
	local obj1 = self:InsertItemText(1,"圈子分组1",nil,root)
	table.insert(GroupsGroup, obj1)
	self:SetExpand(obj1,true)
	self:InsertItemText(1,"test1","兴趣类型1",obj1)
	self:InsertItemText(2,"test2","兴趣类型2",obj1)
	self:InsertItemText(3,"test3","兴趣类型3",obj1)
	self:InsertItemText(4,"test4","兴趣类型4",obj1)
	
	local obj2 = self:InsertItemText(2,"圈子分组2",nil,root)
	table.insert(GroupsGroup, obj2)
	self:SetExpand(obj2,true)
	self:InsertItemText(9,"test9","兴趣类型5",obj2)
	self:InsertItemText(10,"test10","兴趣类型6",obj2)
	self:InsertItemText(11,"test11","兴趣类型7",obj2)
	
	obj1.expand = false
	obj2.expand = false
	self:SetAdjust()
end