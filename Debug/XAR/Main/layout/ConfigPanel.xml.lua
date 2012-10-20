function add_friend_btn_OnClick(self)
    local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local hostWnd = hostwndManager:GetHostWnd("AddFriendFrame")
		
	if not hostWnd then
		local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")
		local frameHostWndTemplate = templateMananger:GetTemplate("AddFriend.Wnd","HostWndTemplate")
		if frameHostWndTemplate then  
			local frameHostWnd = frameHostWndTemplate:CreateInstance("AddFriendFrame")
			if frameHostWnd then
				local objectTreeTemplate = templateMananger:GetTemplate("AddFriend.Tree","ObjectTreeTemplate")
				if objectTreeTemplate then
					local uiObjectTree = objectTreeTemplate:CreateInstance("AddFriendObjectTree")
					if uiObjectTree then
						frameHostWnd:BindUIObjectTree(uiObjectTree)
						frameHostWnd:Create()
					end
				end
			end
		end
	end
end