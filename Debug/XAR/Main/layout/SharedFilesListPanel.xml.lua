function list_OnInitControl(self)
    for i=0, 20 do
	    self:InsertItem(i,"文件名" .. i, i .. "kB/s", i)
	end
	self:UpdateScroll()
	
	--[[
	SetTimer(function ()
	         self:ChangeItemSpeed(1,"kB/s")
			 end,
			 1000)
	--]]
end

function list_OnSelected(self, fun, id)
    XLMessageBox(id)
end

function list_OnPopupItemMenu(self, fun, id)
    local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	if not objtreeManager:GetUIObjectTree("ShareFilesListMenu.Tree.Instance") then
	    local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")				
	    local menuTreeTemplate = templateMananger:GetTemplate("ShareFilesListMenu.Tree","ObjectTreeTemplate")
	    local menuTree = menuTreeTemplate:CreateInstance("ShareFilesListMenu.Tree.Instance")
	    local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")

	    local menuHostWndTemplate = templateMananger:GetTemplate("ShareFilesListMenu.Wnd", "HostWndTemplate")
	    local menuHostWnd = menuHostWndTemplate:CreateInstance("ShareFilesListMenu.Wnd.Instance")
	
	    menuHostWnd:BindUIObjectTree(menuTree)
	
	    local userData = {}
		userData.tree = self
		userData.id = id
	    menuHostWnd:SetUserData(userData)
		
	    local app = XLGetObject("V6Chat3App")
	    local x, y = app:GetCursorPos()
	    menuHostWnd:TrackPopupMenu(hostwnd, x, y, x + 400, y + 400)
		
	    objtreeManager:DestroyTree("ShareFilesListMenu.Tree.Instance")
	    hostwndManager:RemoveHostWnd("ShareFilesListMenu.Wnd.Instance")
	end
end