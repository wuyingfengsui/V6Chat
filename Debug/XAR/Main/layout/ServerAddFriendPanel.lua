function addServer_btn_OnClick(self)
	local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")

	local HostWndTemplate = templateMananger:GetTemplate("AddServer.Wnd","HostWndTemplate")
	local HostWnd = HostWndTemplate:CreateInstance("AddServerWnd")
	local TreeTemplate = templateMananger:GetTemplate("AddServer.Tree","ObjectTreeTemplate")
	local Tree = TreeTemplate:CreateInstance("AddServerTree")
	HostWnd:BindUIObjectTree(Tree)
	HostWnd:DoModal()
	
	local server_data = HostWnd:GetUserData()
	if server_data then
		local app = XLGetObject("V6Chat3App")
		app:AddServer(server_data.server, server_data.admain, server_data.account, server_data.password, server_data.introduction)
	end
	
	local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	objtreeManager:DestroyTree("AddServerTree")
	hostwndManager:RemoveHostWnd("AddServerWnd")
end

function UpdateServerListItem(self, id, name, introduction, domain, state)
	local serverList = self:GetControlObject("serverList")
	serverList:InsertItem(id, name, introduction, domain, state)
	serverList:UpdateScroll()
end

function UpdateUsersListItem(self, serverId, id, account, name, head)
	local usersList = self:GetControlObject("usersList")
	usersList:InsertItem(serverId, id, account, name, head)
	usersList:UpdateScroll()
end

function ShowUsersListEmpty(self)
	local usersList = self:GetControlObject("usersList")
	usersList:ShowText("没有符合条件的用户！")
end

function serverList_OnInitControl(self)
    local app = XLGetObject("V6Chat3App")
	local servers = app:GetServerList()
	
	for i=1,#servers do
		self:InsertItem(servers[i].id, servers[i].server_name, servers[i].introduction, servers[i].domain, servers[i].state)
	end
	
	self:UpdateScroll()
end

function searchContect_OnInitControl(self)
	self:SetText("对方账号")
end

function searchContect_OnFocusChange(self, isFocus, lastFocusObj)
	local text = self:GetText()
	if not isFocus then
		if text == "" then
			self:SetText("对方账号")
		end
	else
		if text == "对方账号" then
			self:SetSelAll()
			self:Clear()
		end
	end
end

function serverList_OnSelected(self, fun, id)
    local control = self:GetOwnerControl()
	local attr = control:GetAttribute()
	attr.selectedId = id
end

function serverList_OnChanged(self, fun, id)
	local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")

	local HostWndTemplate = templateMananger:GetTemplate("AddServer.Wnd","HostWndTemplate")
	local HostWnd = HostWndTemplate:CreateInstance("AddServerWnd")
	local TreeTemplate = templateMananger:GetTemplate("AddServer.Tree","ObjectTreeTemplate")
	local Tree = TreeTemplate:CreateInstance("AddServerTree")
	
	local app = XLGetObject("V6Chat3App")
	local server = app:GetServerInfoByID(id)
	HostWnd:SetUserData(server)
	
	HostWnd:BindUIObjectTree(Tree)
	if HostWnd:DoModal() == 0 then
		local server_data = HostWnd:GetUserData()
		if server_data then
			app:ChangeServerInfo(id, server_data.server, server_data.admain, server_data.account, server_data.password, server_data.introduction)
		end
	end
	
	local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	objtreeManager:DestroyTree("AddServerTree")
	hostwndManager:RemoveHostWnd("AddServerWnd")
end

function usersList_OnAddUser(self, fun, sid, id, account)
	local app = XLGetObject("V6Chat3App")
	app:AddServerFriend(sid, account)
	local uiHelper = XLGetGlobal("V6ChatUIHelper")
	uiHelper:V6MessageBox("请求已发送", 250, 130)
end


function search_btn_OnClick(self)
	local owner = self:GetOwnerControl()
	local searchContect = owner:GetControlObject("searchContect"):GetText()
	local attr = owner:GetAttribute()
	if attr.selectedId == -1 then
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("未选择服务端！", 300, 130)
	elseif searchContect == "" or searchContect == "对方账号" then
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("搜索条件为空！", 300, 130)
	else
		local usersList = owner:GetControlObject("usersList")
		usersList:DeleteAllItems()
		
		local app = XLGetObject("V6Chat3App")
		app:SearchFriendOnServer(attr.selectedId, searchContect)
	end
end

function usersList_OnInitControl(self)
	--for i=0, 20 do
	--    self:InsertItem(i,"昵称" .. i, nil)
	--end
	--self:UpdateScroll()
end