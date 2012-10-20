function my_ip_OnBind(self)
	local app = XLGetObject("V6Chat3App")
	local ipv4, v4port, ipv6, v6port = app:GetMyIpAndPort()
	if ipv4 then
		if ipv4 ~= "" then
			self:AppendText(ipv4)
			self:AppendText(":" .. v4port .. "\r\n")
		end
		if ipv6 ~= "" then
			self:AppendText("[" .. ipv6 .. "]")
			self:AppendText(":" .. v6port)
		end
	end
end

function OK_btn_OnClick(self)
    local owner = self:GetOwnerControl()
	local HostWnd = self:GetOwner():GetBindHostWnd()
	local app = XLGetObject("V6Chat3App")
	local ip_name = owner:GetControlObject("ip")
	local ip = ip_name:GetText()
	local port_name = owner:GetControlObject("port")
	local port = port_name:GetText()
	if ip and ip ~= "" and port and port ~="" then
	    app:AddP2PFriend(ip, port)
		
		local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	    hostwndManager:RemoveHostWnd(HostWnd:GetID())
	    local treeMananger = XLGetObject("Xunlei.UIEngine.TreeManager")
	    treeMananger:DestroyTree(self:GetOwner():GetID())
	else
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("ip或端口为空！", 300, 130)
	end
end

function copy_button_OnClick(self)
	local owner = self:GetOwnerControl()
	local HostWnd = self:GetOwner():GetBindHostWnd()
	local my_ip = owner:GetControlObject("my_ip")
	local app = XLGetObject("V6Chat3App")
	if app:CopyToClipboard(HostWnd:GetID(), my_ip:GetRichText()) then
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("ip及端口已复制到剪贴板中。", 250, 130)
	end
end

function prompt3_OnMouseMove(self)
	self:SetTextFontResID ("font.category.item.underline")
    self:SetCursorID ("IDC_HAND")
end

function prompt3_OnMouseLeave(self)
	self:SetTextFontResID ("font.category.item")
end

function prompt3_OnLButtonDown(self)
end