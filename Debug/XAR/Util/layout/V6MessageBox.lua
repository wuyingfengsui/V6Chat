function Wnd_OnCreate(hwnd)
end

function Wnd_OnShowWindow(self, isShow)
    if isShow then
	    local WndTree = self:GetBindUIObjectTree()
		
		local data = self:GetUserData()
		if data then
			
			if data.width and data.height then
				local bkg = WndTree:GetUIObject("messagebox.bkg")
				bkg:SetObjPos2(0, 0, data.width, data.height)
				self:Move(0, 0, data.width, data.height)
				
				local message = WndTree:GetUIObject("message")
				if data.message then
					message:SetText(data.message)
				end
				
				if (not data.boxtype) or data.boxtype == 1 then
					local OK_button = WndTree:GetUIObject("OK_button")
					OK_button:SetVisible(true)
					OK_button:SetChildrenVisible(true)
				elseif data.boxtype == 2 then
					local Yes_button = WndTree:GetUIObject("Yes_button")
					Yes_button:SetVisible(true)
					Yes_button:SetChildrenVisible(true)
					local No_button = WndTree:GetUIObject("No_button")
					No_button:SetVisible(true)
					No_button:SetChildrenVisible(true)
				end
			end
		end
		
		self:Center()
	end
end

function Wnd_OnSize(self, sizetype, width, height)
	local objectTree = self:GetBindUIObjectTree()
	local close_button = objectTree:GetUIObject("close_button")
	close_button:SetOriginalLeft(width - 30)
end

function destory_Wnd(self, ret)
	local owner = self:GetOwner()
	local HostWnd = owner:GetBindHostWnd()
	
	if ret then
		local data = HostWnd:GetUserData()
		data.ret = ret
		HostWnd:SetUserData(data)
	end
	
	HostWnd:EndDialog(0)
end

function close_btn_OnClick(self)
	destory_Wnd(self)
end

function OK_btn_OnClick(self)
	destory_Wnd(self)
end

function Yes_btn_OnClick(self)
	destory_Wnd(self, 1)
end

function No_btn_OnClick(self)
	destory_Wnd(self, 0)
end