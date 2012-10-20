--lua文件必须是UTF-8编码的(最好无BOM头)
function edit_OnFocusChange(self,isFocus)
	local control = self:GetOwnerControl()
	control:FireExtEvent("OnEditFocusChange", isFocus)
	
	local edit_text = control:GetControlObject("text")
	local edit_bkg = control:GetControlObject("edit.bkg")
	if isFocus then
	    local attr = control:GetAttribute()
		edit_text:SetEffectColorResID(attr.TextColorResId)
		edit_bkg:SetTextureID("texture.edit.bkg.hover")
	else
	    edit_text:SetEffectColorResID("system.transparent")
		edit_bkg:SetTextureID("texture.edit.bkg.normal")
	end
end


function edit_OnEnableChange(self, isEnable)
    local control = self:GetOwnerControl()
	local edit_bkg = control:GetControlObject("edit.bkg")
	if isEnable then
	    edit_bkg:SetTextureID("texture.edit.bkg.normal")
	else
	    edit_bkg:SetTextureID("texture.edit.bkg.disable")
	end
end


function edit_OnKeyUp(self, char, repeatCount, flags)
	local tree = self:GetOwnerControl()
	tree:FireExtEvent("OnTextChanged")
	return 0, true, true
end


function OnBind(self)
    local left,top,right,bottom = self:GetAbsPos()
	local attr = self:GetAttribute()
	local text = self:GetControlObject("text")
	text:SetText(attr.Text)
	text:SetObjPos2(0, 0, attr.TextWidth, bottom - top)
	local edit_bkg = self:GetControlObject("edit.bkg")
	edit_bkg:SetObjPos2(attr.TextWidth, 0, right - left - attr.TextWidth, bottom - top)
	local edit = self:GetControlObject("edit")
	edit:SetIsPassword(attr.Password)
	edit:SetEnable(attr.Enable)
	if not attr.Enable then
	    edit_bkg:SetTextureID("texture.edit.bkg.disable")
	end
end

function OnFocusChange(self,isFocus)
    local edit = self:GetControlObject("edit")
	edit:SetFocus(isFocus)
end

function SetTitle(self, txt)
	local text = self:GetControlObject("text")
	local attr = self:GetAttribute()
	attr.Text = txt
end

function SetTextWidth(self, width)
	local attr = self:GetAttribute()
	attr.TextWidth = width
end

function SetTextColorResId(self, resId)
	local attr = self:GetAttribute()
	attr.TextColorResId = resId
end

function GetText(self)
    local text = self:GetControlObject("edit")
	return text:GetText()
end

function SetText(self, txt)
    local text = self:GetControlObject("edit")
	return text:SetText(txt)
end

function SetIsNumber(self, isNumber)
	local text = self:GetControlObject("edit")
	text:SetIsNumber(isNumber)
end

function SetMaxLength(self, len)
	local text = self:GetControlObject("edit")
	text:SetMaxLength(len)
end