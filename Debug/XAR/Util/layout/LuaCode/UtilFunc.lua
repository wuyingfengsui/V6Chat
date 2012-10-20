function V6MessageBox(self, message, width, height, boxtype)          -- boxtype:ȱʡ��1-��ͨ��ȷ������2-ѡ���Ƿ�
	local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")

	local HostWndTemplate = templateMananger:GetTemplate("V6MessageBox.Wnd","HostWndTemplate")
	local HostWnd = HostWndTemplate:CreateInstance("V6MessageBoxWnd")
	local TreeTemplate = templateMananger:GetTemplate("V6MessageBox.Tree","ObjectTreeTemplate")
	local Tree = TreeTemplate:CreateInstance("V6MessageBoxTree")
	
	local parms = {}
	parms.message = message
	parms.width = width
	parms.height = height
	parms.boxtype = boxtype
	HostWnd:SetUserData(parms)
	
	local ret = 0
	HostWnd:BindUIObjectTree(Tree)
	if HostWnd:DoModal() == 0 then
		ret = HostWnd:GetUserData().ret
	end
	
	local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	objtreeManager:DestroyTree("V6MessageBoxTree")
	hostwndManager:RemoveHostWnd("V6MessageBoxWnd")
	
	return ret
end

function RegisterObject(self)
	local UIHelper = {}
	UIHelper.V6MessageBox = V6MessageBox
	XLSetGlobal("V6ChatUIHelper",UIHelper)
end