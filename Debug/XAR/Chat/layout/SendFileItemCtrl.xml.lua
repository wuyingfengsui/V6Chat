function SetId(self, id)
	local attr = self:GetAttribute()
	attr.id = id
end

function GetId(self)
	local attr = self:GetAttribute()
	return attr.id
end

function SetState(self, state)
	local attr = self:GetAttribute()
	attr.state = state
	
	if attr.state == 1 then
		local file_name = self:GetControlObject("file_name")
		file_name:SetText(attr.file_name)
		file_name:SetVisible(true)
		
		local open_file = self:GetControlObject("open_file")
		open_file:SetText("打开文件")
		local open_file_path = self:GetControlObject("open_file_path")
		open_file_path:SetVisible(true)
	elseif attr.state == 2 then
		local open_file = self:GetControlObject("open_file")
		open_file:SetText("传输失败")
	else
		local open_file = self:GetControlObject("open_file")
		open_file:SetText("文件传输中")
	end
end

function SetFileName(self, fileName)
	local attr = self:GetAttribute()
	attr.file_name = fileName
end

function SetFilePath(self, filePath)
	local attr = self:GetAttribute()
	attr.file_path = filePath
end

function OnMouseMove(self)
    local attr = self:GetOwnerControl():GetAttribute()
	if attr.state==1 then
	    self:SetCursorID ("IDC_HAND")
		self:SetTextFontResID("font.category.item.underline")
    end
end


function OnMouseLeave(self)
    local attr = self:GetOwnerControl():GetAttribute()
	if attr.state==1 then
		self:SetCursorID ("IDC_ARROW")
		self:SetTextFontResID("font.category.item")
	end
end

function open_file_OnLButtonDown(self, x, y, flags)
	local attr = self:GetOwnerControl():GetAttribute()
	local app = XLGetObject("V6Chat3App")
	app:OpenShellExecute(attr.file_path .. attr.file_name)
end

function open_file_path_OnLButtonDown(self, x, y, flags)
	local attr = self:GetOwnerControl():GetAttribute()
	local app = XLGetObject("V6Chat3App")
	app:OpenShellExecute(attr.file_path)
end

function OnInitControl(self)
	local attr = self:GetAttribute()
	
	if attr.state == 1 then
		local file_name = self:GetControlObject("file_name")
		file_name:SetText(attr.file_name)
		file_name:SetVisible(true)
		
		local open_file = self:GetControlObject("open_file")
		open_file:SetText("打开文件")
		local open_file_path = self:GetControlObject("open_file_path")
		open_file_path:SetVisible(true)
	else
		local open_file = self:GetControlObject("open_file")
		open_file:SetText("文件传输中")
	end
end

function OnLButtonDbClick(self)
	local attr = self:GetAttribute()
	--os.execute(attr.file_path .. attr.file_name)
end