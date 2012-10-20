function tree_OnHover(self, fun, item, y)
	local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("InfoTips")
	if not tipsHostWnd then
		local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
		local tipsHostWndTemplate = templateMananger:GetTemplate("Main.Info.TipsWnd","HostWndTemplate")
		tipsHostWnd = tipsHostWndTemplate:CreateInstance("InfoTips")
	end
	tipsHostWnd:SetTipTemplate("Main.UserInfo.Tips")
	
	local friendsListHostWnd = hostWndManager:GetHostWnd("MainTab1")
	local wnd_left,wnd_top,wnd_right,wnd_bottom = friendsListHostWnd:GetWindowRect()
	if wnd_left>278 then
		tipsHostWnd:SetPositionByWindow(-278,y,friendsListHostWnd:GetWndHandle())
	else
		tipsHostWnd:SetPositionByWindow(wnd_right - wnd_left + 10,y,friendsListHostWnd:GetWndHandle())
	end
		
	local app = XLGetObject("V6Chat3App")
	local userData = app:GetUserInfoByID(item.id)
	if userData then
	    tipsHostWnd:SetUserData(userData)
	end
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
	local XARManager = XLGetObject("Xunlei.UIEngine.XARManager")
	local loaded = XARManager:IsXARLoaded("Chat")
	if not loaded then
		loaded = XARManager:LoadXAR("Chat")
	end
	
	if loaded then
		local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
		local hostWnd = hostwndManager:GetHostWnd("ChatFrame" .. item.id)
		
		if not hostWnd then
			local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
			local frameHostWndTemplate = templateMananger:GetTemplate("Chat.Wnd","HostWndTemplate")
			if frameHostWndTemplate then  
				local frameHostWnd = frameHostWndTemplate:CreateInstance("ChatFrame" .. item.id)
				if frameHostWnd then
					local objectTreeTemplate = templateMananger:GetTemplate("Chat.Tree","ObjectTreeTemplate")
					if objectTreeTemplate then
						local uiObjectTree = objectTreeTemplate:CreateInstance("ChatObjectTree" .. item.id)
						if uiObjectTree then
							frameHostWnd:BindUIObjectTree(uiObjectTree)
							frameHostWnd:SetUserData(item.id)
							frameHostWnd:Create()
						end
					end
				end
			end
		end
	end
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
		userData.groups = friendsGroup
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
	if not objtreeManager:GetUIObjectTree("FriendsListMenu.Tree.Instance") then
	    local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")				
	    local menuTreeTemplate = templateMananger:GetTemplate("FriendsListMenu.Tree","ObjectTreeTemplate")
	    local menuTree = menuTreeTemplate:CreateInstance("FriendsListMenu.Tree.Instance")
	    local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")

	    local menuHostWndTemplate = templateMananger:GetTemplate("FriendsListMenu.Wnd", "HostWndTemplate")
	    local menuHostWnd = menuHostWndTemplate:CreateInstance("FriendsListMenu.Wnd.Instance")
	
	    menuHostWnd:BindUIObjectTree(menuTree)
	
	    local userData = {}
		userData.tree = self
		userData.item = item
		userData.groups = friendsGroup
	    menuHostWnd:SetUserData(userData)
		
	    local app = XLGetObject("V6Chat3App")
	    local x, y = app:GetCursorPos()
	    menuHostWnd:TrackPopupMenu(hostwnd, x, y, x + 400, y + 400)
		
	    objtreeManager:DestroyTree("FriendsListMenu.Tree.Instance")
	    hostwndManager:RemoveHostWnd("FriendsListMenu.Wnd.Instance")
	end
end

function InsertFriend(grouptree, id, nick_name, location, group_name, head_image)
    local root = grouptree:GetRootItem()
	local newFriendTree = grouptree:GetChildItemByTxt(root, group_name)
	if not newFriendTree then
		newFriendTree = grouptree:InsertItemText(grouptree:GetChildrenItemCount(root)+1,group_name,nil,root)
		table.insert(friendsGroup, newFriendTree)
	end
	local newFriend = grouptree:GetChildItemById(newFriendTree, id)
	if not newFriend then
	    local obj = grouptree:InsertItemImageText(id,head_image,nick_name,location,newFriendTree)
	else
	    grouptree:UpdateChildItem(id,head_image,nick_name,location,group_name)
	end
end

function OffFriend(id, group_name)
    local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	if treeManager then
	    local friendList = treeManager:GetUIObjectTree("MainTab1Tree")
		if friendList then
	        local friendTree = friendList:GetUIObject("tree")
			if friendTree then
		        friendTree:OffChildItem(id,nil,"离线",group_name)
			    friendTree:SetAdjust()
			end
		end
	end
end

function tree_OnInitControl(self)
    local root = self:GetRootItem()
	local app = XLGetObject("V6Chat3App")
	friendsGroup = {}
	local friends = app:GetUserList()
	
	for i=1,#friends do
		InsertFriend(self, friends[i].id, friends[i].nickname, friends[i].location, friends[i].groupname, friends[i].head)
	end
	self:SetAdjust()
	
	app:StartServer()
end

function wnd_OnCreate(hostwnd)
	local function MessageFilter(self, msg, wparam, lparam)
		if msg == 0x0401 then               --更新好友列表事件
		    if wparam == 3 then             --更新好友
				local app = XLGetObject("V6Chat3App")
				local user = app:GetUserInfoByID(lparam)
				if user then
				    local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
					local friendList = treeManager:GetUIObjectTree("MainTab1Tree")
					local friendTree = friendList:GetUIObject("tree")
					InsertFriend(friendTree, lparam, user.name, user.location, user.groupname, user.head)
					friendTree:SetAdjust()
				end
			elseif wparam == 4 then         --好友下线
			    local app = XLGetObject("V6Chat3App")
				local id, group_name = app:OffFriend(lparam)
				if id then
				    OffFriend(id, group_name)
				end
			end
		elseif msg == 0x0402 then        --更新聊天内容
			local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
			local hostWnd = hostwndManager:GetHostWnd("ChatFrame" .. lparam)
		
			if not hostWnd then
				local XARManager = XLGetObject("Xunlei.UIEngine.XARManager")
				local loaded = XARManager:IsXARLoaded("Chat")
				if not loaded then
					loaded = XARManager:LoadXAR("Chat")
				end
	
				if loaded then
					local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
					local frameHostWndTemplate = templateMananger:GetTemplate("Chat.Wnd","HostWndTemplate")
					if frameHostWndTemplate then  
						local frameHostWnd = frameHostWndTemplate:CreateInstance("ChatFrame" .. lparam)
						if frameHostWnd then
							local objectTreeTemplate = templateMananger:GetTemplate("Chat.Tree","ObjectTreeTemplate")
							if objectTreeTemplate then
								local uiObjectTree = objectTreeTemplate:CreateInstance("ChatObjectTree" .. lparam)
								if uiObjectTree then
									frameHostWnd:BindUIObjectTree(uiObjectTree)
									frameHostWnd:SetUserData(lparam)
									frameHostWnd:Create()
								end
							end
						end
					end
				end
			end
		end
	end
	
	local cookie, ret = hostwnd:AddInputFilter(nil, MessageFilter)
	friendList_wnd_cookie = cookie
end

function wnd_OnDestroy(hostwnd)
    hostwnd:RemoveInputFilter(friendList_wnd_cookie)
end

function wnd_OnShowWindow(self, isShow)
    if isShow then
	end
end