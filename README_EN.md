ExDUIR
=======
[中文](./README.md)

## Introduction

Lightweight DirectUI framework for Windows platform.

This project is based on ExDirectUI 4.1.18 open source.

## Improvements over the original

Please refer to[update log](CHANGELOG.txt)

## Problem feedback & Help
* AC QQ group::214406227
* Submit feedback:[Gitee issues](https://gitee.com/william_lzw/ExduiR/issues) or [GitHub issues](https://github.com/laizewei/ExduiR/issues)

## Environment
win10+

win7 need install KB2670838 patch

VMware win7 run ExDUIR, install kb2670838 and kb4474419 patches, and then install VMware Tools.

## Document
* [Document](doc/SUMMARY.md)

## Compiler
* Visual Studio 2022(recommended)
  ExDUIR.sln。Includes the following items:
  * dll: Compile to a dynamic library. -> libexdui.dll
  * libexdui: Compile to a static library.  -> libexdui.lib
  * libexduiCPP: C++ class encapsulation and call demonstration.
  * test: Test project.


## Characteristics
* Rendering function is DirectX11, Direct2D, efficiency is hundreds of times higher than GDI.
* Unicode encoding, support for multiple languages.
* Mimic The Windows messaging mechanism by sending messages and distributing notifications to components, compatible with native Windows messages and constants. Win32 API writing, more flexible than class writing extensibility.
* Use Win32 style API method to call, support the generation of lib static library and DLL dynamic library. The generated dynamic link library can be called by other languages such as python, Java, go, dephi, C#, VB, easy language, etc.
* Extension components are free and flexible, and the engine handles the underlying logic. The user only needs to write the drawing code and does not need to worry about complex message processing.
* Currently support component has a button, switch, radio buttons, select box, edit box, rich text edit box, listview, report listview, template listview, group boxes, combo boxes, menus, tree frame, the slider, radio buttons, select box, TAB, load, animations, rotating picture box, page, picture box, message box, list of ICONS, list button, the toolbar , status bar, date box, color palette, color picker, title box, calendar box, score button, cef3 browser box, drawing board, media player, roll menu, tagging board, you can superclass extension components on these components.
* Support for layouts that automatically update component positions when window sizes change. Currently, the following layouts are supported: absolute layout, relative layout, linear layout, flow layout, and table layout. Users can extend the layout.
* Support GIF format images automatically play animation, support image format PNG, JPEG, BMP, GIF, DDS, ICON. Windows or components can use the above format image as the background.
* Support 34 eases, users can freely write window eases or component eases. Users can extend the easing algorithm.
* Window support special-shaped window, picture shape window. Windows and components support 0 to 255 transparency. Supports Gaussian blur.
* Edit box supports rich text and supports loading documents in RTF format.
* Windows or components can receive drag files or text.
* Support for modal Windows.
* Support for restricted area message notification.

## Window Drawing Concepts
* The graphical interface is based on windows and a messaging mechanism. Controls are drawn on the window and respond to messages like mouse and keyboard to implement functionality. For example, when the mouse hovers over a button, clicking it will trigger the button click event and change the button's rendered state to pressed. Window and control drawing uses either GDI or DirectX, the latter being much faster by using GPU hardware acceleration.
* ExDuiR uses DirectX (DirectX 11 and Direct2D) for drawing to leverage GPU acceleration, and subclasses windows to enable custom drawing and message handling.

## Overall Design
* ExDuiR exposes APIs as functions to allow compiling into DLLs for use by other languages.
* ExDuiR includes custom base classes like Array, HandleTable, HashTable, Theme, Resource, Easing, ImageList, Layout, MemPool, Thread, DropTarget.
* Drawing related base classes include Brush, Canvas, Font, Image, Matrix, Path, Region, StrokeStyle.

## Extension component description
Ex_ObjRegister registers the component class, draws the component in the WM_PAINT message in the callback, and other messages control the state of the component. Create a component with the class name. The number of properties required by a component is set in the cbObjextra of Ex_ObjRegister, and each has a default size of 8 bytes. Use an index that starts at 0 (negative numbers are built-in indexes for the underlying component, do not use). Use Ex_ObjSetLong to set the properties and Ex_ObjGetLong to get the properties.

* It is possible to superclass an extension component to class_rotateImageBox_ex.cpp on top of the underlying component callbacks, which return Ex_ObjCallProc by default
* You can recustomize the component callback extension component to refer to class_titlebar_ex.cpp. By default, the callback returns Ex_ObjDefProc
The drawing component starts by calling Ex_ObjBeginPaint, passing in the drawing structure, and ends by calling Ex_ObjEndPaint.

## Demo
### demo_all:  
![image](demo_image/demo_all.png)    
### demo button and switch:  
![image](demo_image/demo_button_switch.png)    
### demo buttonex:  
![image](demo_image/demo_buttonex.png)    
### demo combobox:  
![image](demo_image/demo_combobox.png)       
### demo custom menu:  
![image](demo_image/demo_custommenu.png)    
### demo custom background:  
![image](demo_image/demo_custombackground.png)    
### demo drag obj:  
![image](demo_image/demo_drag_obj.png)    
### demo drop and drag:  
![image](demo_image/demo_dropdrag.png)    
### demo easing: 
![image](demo_image/demo_easing.png)    
### demo edit and richedit:  
![image](demo_image/demo_edit_richedit.png)   
### demo editex:  
![image](demo_image/demo_editex.png)    
### demo groupbox:  
![image](demo_image/demo_groupbox.png)    
### demo iconlistview:  
![image](demo_image/demo_iconlistview.png)      
### demo irregular shape window:  
![image](demo_image/demo_irregular_shape_window.png)    
### demo label and background:  
![image](demo_image/demo_label_background.png)   
### demo layout-absolute:  
![image](demo_image/demo_layout_absolute.png)    
### demo layout-flow:  
![image](demo_image/demo_layout_flow.png)    
### demo layout-linear:  
![image](demo_image/demo_layout_linear.png)    
### demo layout-relative:  
![image](demo_image/demo_layout_relative.png)    
### demo layout-table:  
![image](demo_image/demo_layout_table.png)
### demo listview:  
![image](demo_image/demo_listview.png)
### demo loading:  
![image](demo_image/demo_loading.png)
### demo matrix:  
![image](demo_image/demo_matrix.png)
### demo menubutton:  
![image](demo_image/demo_menubutton.png)
### demo messagebox:  
![image](demo_image/demo_messagebox.png)
### demo navbutton:  
![image](demo_image/demo_navbutton.png)
### demo nchitest:  
![image](demo_image/demo_nchitest.png)
### demo progressbar:  
![image](demo_image/demo_progressbar.png)
### demo radio and checkbox:  
![image](demo_image/demo_radio_checkbox.png)
### demo reportlistview:  
![image](demo_image/demo_reportlistview.png)
### demo rotatebox:  
![image](demo_image/demo_rotatebox.png)
### demo sliderbar:  
![image](demo_image/demo_sliderbar.png)
### demo treeview:  
![image](demo_image/demo_treeview.png)
### demo modal:  
![image](demo_image/demo_modal.png)
### demo titlebar:
![image](demo_image/demo_titlebar.png)
### demo datebox:
![image](demo_image/demo_datebox.png)
### demo palette:
![image](demo_image/demo_palette.png)
### demo color picker:
![image](demo_image/demo_colorpicker.png)
### demo calendar:
![image](demo_image/demo_calendar.png)
### demo cefbrowser:
![image](demo_image/demo_cefbrowser.png)
### demo score button:
![image](demo_image/demo_scorebutton.png)
### demo template listview:
![image](demo_image/demo_template_listview.png)
### demo drawing board:
![image](demo_image/demo_drawingboard.png)
### demo propertygrid:
![image](demo_image/demo_propertygrid.png)
### demo media player:
![image](demo_image/demo_mediaplayer.png)
### demo svg:
![image](demo_image/demo_svg.png)
### demo carousel:
![image](demo_image/demo_carousel.png)
### demo rollmenu:
![image](demo_image/demo_rollmenu.png)
### demo tray:
![image](demo_image/demo_trayicon.png)
### demo mask:
![image](demo_image/demo_mask.png)
### demo taggingboard:
![image](demo_image/demo_taggingboard.png)
### demo effect:
![image](demo_image/demo_effect.png)
### demo pathandrgn:
![image](demo_image/demo_pathandrgn.png)
### demo chatbox:
![image](demo_image/demo_chatbox.png)