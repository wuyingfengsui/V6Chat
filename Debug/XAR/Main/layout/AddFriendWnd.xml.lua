function Close_btn_OnClick(self)
	local owner = self:GetOwner()
	local HostWnd = owner:GetBindHostWnd()
	local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	hostwndManager:RemoveHostWnd(HostWnd:GetID())
	local treeMananger = XLGetObject("Xunlei.UIEngine.TreeManager")
	treeMananger:DestroyTree(owner:GetID())
end

function OnCreate(self)
    local function MessageFilter(self, msg, wparam, lparam)
		if msg == 0x0401 then               --更新服务器列表事件
		    if wparam == 3 then             --更新服务器
				local app = XLGetObject("V6Chat3App")
				local server = app:GetServerInfoByID(lparam)
				if server then
				    local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
					local addFriendList = treeManager:GetUIObjectTree("AddFriendObjectTree")
					local ServerPanel = addFriendList:GetUIObject("ServerPanel")
					ServerPanel:UpdateServerListItem(server.id, server.server_name, server.introduction, server.domain, server.state)
				end
			end
		elseif msg == 0x0402 then           --更新搜索用户结果
			local app = XLGetObject("V6Chat3App")
			local resultTable = app:GetSearchFriendResult(lparam)
			if resultTable then
				local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
				local addFriendList = treeManager:GetUIObjectTree("AddFriendObjectTree")
				local ServerPanel = addFriendList:GetUIObject("ServerPanel")
				if #resultTable == 0 then
					ServerPanel:ShowUsersListEmpty()
				else
					for i=1,#resultTable do
						ServerPanel:UpdateUsersListItem(lparam, resultTable[i].id, resultTable[i].account, resultTable[i].formatted_name, resultTable[i].head)
					end
				end
			end
		end
	end
	
	local cookie, ret = self:AddInputFilter(nil, MessageFilter)
	
	self:Center()
end

function OnShowWindow(self, isShow)
    if isShow then
		local tree = self:GetBindUIObjectTree()
		
		local p2pPanel = tree:GetUIObject("P2PPanel")
		p2pPanel:SetVisible(false)
		p2pPanel:SetChildrenVisible(false)
		local serverPanel = tree:GetUIObject("ServerPanel")
		serverPanel:SetVisible(false)
		serverPanel:SetChildrenVisible(false)
	end
end

function hide_main_panel(self)
	local tree = self:GetOwner()
	
	local prompt1 = tree:GetUIObject("prompt1")
	prompt1:SetVisible(false)
	local prompt2 = tree:GetUIObject("prompt2")
	prompt2:SetVisible(false)
	local p2pBtn = tree:GetUIObject("p2p_button")
	p2pBtn:SetVisible(false)
	p2pBtn:SetChildrenVisible(false)
	local prompt3 = tree:GetUIObject("prompt3")
	prompt3:SetVisible(false)
	local serverBtn = tree:GetUIObject("server_button")
	serverBtn:SetVisible(false)
	serverBtn:SetChildrenVisible(false)
end

function p2p_button_OnClick(self)
	hide_main_panel(self)
	
	local tree = self:GetOwner()
	
	local p2pPanel = tree:GetUIObject("P2PPanel")
	p2pPanel:SetVisible(true)
	p2pPanel:SetChildrenVisible(true)
end

function server_button_OnClick(self)
	hide_main_panel(self)
	
	local tree = self:GetOwner()
	
	local serverPanel = tree:GetUIObject("ServerPanel")
	serverPanel:SetVisible(true)
	serverPanel:SetChildrenVisible(true)
end