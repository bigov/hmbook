# Нумерованый список

Код ниже демонстрирует создание нумерованного списка с автоматической нумерацией строк выделеного диапазона.

```cpp
        auto def_num = new wxRichTextListStyleDefinition("NumberedList");
        def_num->SetAttributes(0, 30, 60, wxTEXT_ATTR_BULLET_STYLE_ARABIC | wxTEXT_ATTR_BULLET_STYLE_PERIOD);
        this->style_sheet->AddListStyle(def_num);

        this->Freeze();
        this->Clear();
        this->WriteText("First row.\n");
        this->WriteText("Second row.\n");
        this->WriteText("Threed row.\n");
        this->Thaw();
        this->SetSelection(0, this->GetLastPosition());
        int n_start = 1; // с какого знчения начинать нумерацию строк списка
        
        // вариант 1 
/*      this->SetListStyle(this->GetSelectionRange(), "NumberedList",
            wxRICHTEXT_SETSTYLE_WITH_UNDO | wxRICHTEXT_SETSTYLE_RENUMBER, n_start, -1 ); */
        // вариант 2
        this->NumberList(this->GetSelectionRange(), "NumberedList",
            wxRICHTEXT_SETSTYLE_WITH_UNDO | wxRICHTEXT_SETSTYLE_RENUMBER, n_start, -1 );
            
        this->SetSelection(0, 0); // Снять выделение
        return;
```

Результат (оба варианта - один результат):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<richtext version="1.0.0.0" xmlns="http://www.wxwidgets.org">
  <paragraphlayout textcolor="#444444" bgcolor="#FFFFFF" fontpointsize="11" fontfamily="74" fontstyle="90" fontweight="400" fontunderlined="0" fontface="Adwaita Sans Text" characterstyle="CharBase" leftindent="10" leftsubindent="0" rightindent="8" linespacing="0">
    <paragraph leftindent="30" leftsubindent="60" bulletstyle="257" bulletnumber="1" liststyle="NumberedList">
      <text>First row.</text>
    </paragraph>
    <paragraph leftindent="30" leftsubindent="60" bulletstyle="257" bulletnumber="2" liststyle="NumberedList">
      <text>Second row.</text>
    </paragraph>
    <paragraph leftindent="30" leftsubindent="60" bulletstyle="257" bulletnumber="3" liststyle="NumberedList">
      <text>Threed row.</text>
    </paragraph>
    <paragraph>
      <text></text>
    </paragraph>
  </paragraphlayout>
</richtext>
```

