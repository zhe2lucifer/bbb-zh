#================================================================================================================
#
#  test_CTS.sh
#
#    This script is used to test HDMI CTS.
#
#    Usage:
#        ./test_CTS.sh
#
#
#    Description:
#        This script will 
#            (i) call application "./vpotest" to change TV system, then
#            (ii) call application "./videotest" to playback stream
#
#
#    Note: 
#
#================================================================================================================
#!/bin/sh

vpotest_bin="./alislhdmikit"
#vpotest_bin="./vpotest"
#videotest_bin="./videotest"
videotest_bin="./alislhdmikit"
no_DRM=1
Video_PlayBack_pid=0
usb_partion_is_sda=0
usb_partion_is_sda1=0
usb_partion_is_sda2=0
usb_partion_is_sda3=0
usb_dir=""
format_4K_444=0
##############################################################################
#                                   Functions
##############################################################################
#------------------------------------------------------
#              function: aui_dis_cur()
#------------------------------------------------------
aui_dis_cur () {

if [ -f /tmp/dis_change_tv.txt ];then
	rm -f /tmp/dis_change_tv.txt
fi

}

#------------------------------------------------------
#              function: aui_dis_change_tv()
#------------------------------------------------------
aui_dis_change_tv () {

$vpotest_bin --disp  tvsysmode  $DIS_mode > /dev/null 2>&1

}

#------------------------------------------------------
#              function: Video_PlayBack()
#------------------------------------------------------
Video_PlayBack () {

if [ "$usb_partion_is_sda" == "1" ];then
	usb_dir="sda"
elif [ "$usb_partion_is_sda1" == "1" ];then
	usb_dir="sda1"
elif [ "$usb_partion_is_sda2" == "1" ];then
	usb_dir="sda2"
elif [ "$usb_partion_is_sda3" == "1" ];then
	usb_dir="sda3"
else
	"Cound not find the usb mounted point!!"
	 return 0
fi

echo "usb_dir="$usb_dir

if [ "$no_DRM" == "1" ]; then

echo "Play without DRM"
play_video_file=/mnt/usb/$usb_dir/av_inject/Sony_4K_Surfing.mp4
$videotest_bin -v   $play_video_file  0 h265 3840 2160 60 aac 48000  > /dev/null 2>&1 &

else 

echo "Play with DRM"
play_video_file=/mnt/usb/$usb_dir/av_inject/SamS.ts
$videotest_bin -v  $play_video_file 0 h265  3840  2160  24  aac  48000 > /dev/null 2>&1 &

fi

Video_PlayBack_pid=$!
}

#------------------------------------------------------
#              function: Video_Stop()
#------------------------------------------------------
Video_Stop () {

  if [ "$Video_PlayBack_pid" != "0" ]; then
   kill -2 $Video_PlayBack_pid
   Video_PlayBack_pid=0
  fi
}


#------------------------------------------------------
#              function: ChangeTV()
#------------------------------------------------------
ChangeTV () {
$vpotest_bin --hdmi wr_reg 0x0a 0x02 > /dev/null  2>&1
$vpotest_bin --hdmi set_deep_color 0 > /dev/null  2>&1

aui_dis_change_tv

if [ "$format_4K_444" == "1" ]; then
  echo ">>>> format_4k_444 ="$format_4K_444
  $vpotest_bin -d out_pic_fmt 1 > /dev/null 2>&1

fi

$vpotest_bin --hdmi set_deep_color $DP_mode > /dev/null 2>&1

}



#------------------------------------------------------
#              function: Display_PHY_setting()
#------------------------------------------------------
Display_PHY_setting() {
echo ""
echo ""
echo "************************"
echo "     Sub menu : PHY setting"
echo "************************"
echo "Usage:"
echo "   dis item/all: display item's or all items' value "
echo "   set item val: modify item's value"
echo "   pattern on/ff: output test pattern"
echo "   scramble normal/on/off: set scramble mode(normal, force on, force off>"
echo "              q: quit to main menu"
echo "         others: display usage"
echo "ex."
echo "  dis all"
echo "  dis FB_SEL"
echo "  set FB_SEL 0x17"
echo "  pattern on"
echo "  pattern off"
echo "  scramble on"
echo "  scramble off"
echo "  scramble normal"
echo ""
echo ""
}


#------------------------------------------------------
#              function: Display_TV_selection()
#------------------------------------------------------
Display_TV_selection() {
echo ""
echo ""
echo "************************"
echo "     Sub menu : choose TV mode"
echo "************************"
echo "enter new mode for test:"
echo "      O:  480p60, 8bit(27MHz)              10O:  576p50, 8bit(27MHz)"               
echo "      1:  480p60,10bit(33.75MHz)           101:  576p50,10bit(33.75MHz)"            
echo "      2:  480p60,12bit(40.5MHz)            102:  576p50,12bit(40.5MHz)"             
echo "      3:  480p60,16bit(54MHz)              103:  576p50,16bit(54MHz)"               
echo "      4:  720p60, 8bit(74.25MHz)           104:  720p50, 8bit(74.25MHz)"            
echo "      5:  720p60,10bit(92.8125MHz)         105:  720p50,10bit(92.8125MHz)"          
echo "      6:  720p60,12bit(111.375MHz)         106:  720p50,12bit(111.375MHz)"          
echo "      7:  720p60,16bit(148.5MHz)           107:  720p50,16bit(148.5MHz)"            
echo "      8: 1080p60, 8bit(148.5MHz)           108: 1080p50, 8bit(148.5MHz)"            
echo "      9: 1080p60,10bit(185.625MHz)         109: 1080p50,10bit(185.625MHz)"          
echo "     10: 1080p60,12bit(222.75MHz)          110: 1080p50,12bit(222.75MHz)"           
echo "     11: 1080p60,16bit(297MHz)             111: 1080p50,16bit(297MHz)"              
echo "     12: 4K(3840p60,420), 8bit(297MHz)     112: 4K(3840p50,420), 8bit(297MHz)"      
echo "     13: 4K(3840p60,420),10bit(371.25MHz)  113: 4K(3840p50,420),10bit(371.25MHz)"   
echo "     14: 4K(3840p60,420),12bit(445.5MHz)   114: 4K(3840p50,420),12bit(445.5MHz)"    
echo "     15: 4K(3840p60,420),16bit(594MHz)     115: 4K(3840p50,420),16bit(594MHz)"      
echo "     16: 4K(3840p30,444), 8bit(297MHz)     116: 4K(3840p25,444), 8bit(297MHz)" 
echo "     17: 4K(3840p30,444),10bit(371.25MHz)  117: 4K(3840p25,444),10bit(371.25MHz)"   
echo "     18: 4K(3840p30,444),12bit(445.5MHz)   118: 4K(3840p25,444),12bit(445.5MHz)"    
echo "     19: 4K(3840p30,444),16bit(594MHz)     119: 4K(3840p25,444),16bit(594MHz)"        
echo "     20: 4K(4096p60,420), 8bit(297MHz)     120: 4K(4096p50,420), 8bit(297MHz)"      
echo "     21: 4K(4096p60,420),10bit(371.25MHz)  121: 4K(4096p50,420),10bit(371.25MHz)"   
echo "     22: 4K(4096p60,420),12bit(445.5MHz)   122: 4K(4096p50,420),12bit(445.5MHz)"    
echo "     23: 4K(4096p60,420),16bit(594MHz)     123: 4K(4096p50,420),16bit(594MHz)"  
echo "     24: 4K(4096p30,444), 8bit(297MHz)     124: 4K(4096p25,444), 8bit(297MHz)"      
echo "     25: 4K(4096p30,444),10bit(371.25MHz)  125: 4K(4096p25,444),10bit(371.25MHz)"   
echo "     26: 4K(4096p30,444),12bit(445.5MHz)   126: 4K(4096p25,444),12bit(445.5MHz)"    
echo "     27: 4K(4096p30,444),16bit(594MHz)     127: 4K(4096p25,444),16bit(594MHz)"          
echo "     28: 4K(3840p60,444), 8bit(594MHz)     128: 4K(3840p50,444), 8bit(594MHz)"      
echo "     29: 4K(4096p60,444), 8bit(594MHz)     129: 4K(4096p50,444), 8bit(594MHz)" 
echo "     212: 4K(3840p60,420), with HDR"   
echo ""
echo ""
echo "      q: quit to main menu"
echo " others: display selection"
echo ""
echo ""
}

#------------------------------------------------------
#              function: Choose_mode()
#------------------------------------------------------
Choose_mode() {
choose_ret=1
format_4K_444=0

case $PHY_mode in
  # ---- N type ----
  "0")
    select_msg="select   O:  480p, 8bit(27MHz)"
    TV_mode=480p
    DP_mode=0
    DIS_mode=8
    no_DRM=1
    ;;
  "1")
    select_msg="select    1:  480p,10bit(33.75MHz)"
    TV_mode=480p
    DP_mode=1
    DIS_mode=8
    no_DRM=1
    ;;
  "2")
    select_msg="select    2:  480p,12bit(40.5MHz)"
    TV_mode=480p
    DP_mode=2
    DIS_mode=8
    no_DRM=1
    ;;
  "3")
    select_msg="select    3:  480p,16bit(54MHz)"
    TV_mode=480p
    DP_mode=3
    DIS_mode=8
    no_DRM=1
    ;;
  "4")
    select_msg="select    4:  720p60, 8bit(74.25MHz)"
    TV_mode=720p60
    DP_mode=0
    DIS_mode=10
    no_DRM=1
    ;;
  "5")
    select_msg="select    5:  720p60,10bit(92.8125MHz)"
    TV_mode=720p60
    DP_mode=1
    DIS_mode=10
    no_DRM=1
    ;;
  "6")
    select_msg="select    6:  720p60,12bit(111.375MHz)"
    TV_mode=720p60
    DP_mode=2
    DIS_mode=10
    no_DRM=1
    ;;
  "7")
    select_msg="select    7:  720p60,16bit(148.5MHz)"
    TV_mode=720p60
    DP_mode=3
    DIS_mode=10
    no_DRM=1
    ;;
  "8")
    select_msg="select    8: 1080p60, 8bit(148.5MHz)"
    TV_mode=1080p60
    DP_mode=0
    DIS_mode=17
    no_DRM=1
    ;;
  "9")
    select_msg="select    9: 1080p60,10bit(185.625MHz)"
    TV_mode=1080p60
    DP_mode=1
    DIS_mode=17
    no_DRM=1
    ;;
  "10")
    select_msg="select   10: 1080p60,12bit(222.75MHz)"
    TV_mode=1080p60
    DP_mode=2
    DIS_mode=17
    no_DRM=1
    ;;
  "11")
    select_msg="select   11: 1080p60,16bit(297MHz)"
    TV_mode=1080p60
    DP_mode=3
    DIS_mode=17
    no_DRM=1
    ;;
  "12")
    select_msg="select   12: 4K(3840p60,420), 8bit(297MHz)"
    TV_mode=3840p60
    DP_mode=0
    DIS_mode=23
    no_DRM=1
    ;;  
  "13")
    select_msg="select   13: 4K(3840p60,420),10bit(371.25MHz)"
    TV_mode=3840p60
    DP_mode=1
    DIS_mode=23
    no_DRM=1
    ;;
  "14")
    select_msg="select   14: 4K(3840p60,420),12bit(445.5MHz)"
    TV_mode=3840p60
    DP_mode=2
    DIS_mode=23
    no_DRM=1
    ;;
  "15")
    select_msg="select   15: 4K(3840p60,420),16bit(594MHz)"
    TV_mode=3840p60
    DP_mode=3
    DIS_mode=23
    no_DRM=1
    ;;
  "16")
    select_msg="select   16: 4K(3840p30,444), 8bit(297MHz)"
    TV_mode=3840p30
    DP_mode=0
    DIS_mode=21
    no_DRM=1
    ;;
  "17")
    select_msg="select   17: 4K(3840p30,444),10bit(371.25MHz)"
    TV_mode=3840p30
    DP_mode=1
    DIS_mode=21
    no_DRM=1
    ;;
  "18")
    select_msg="select   18: 4K(3840p30,444),12bit(445.5MHz)"
    TV_mode=3840p30
    DP_mode=2
    DIS_mode=21
    no_DRM=1
    ;;
  "19")
    select_msg="select   19: 4K(3840p30,444),16bit(594MHz)"
    TV_mode=3840p30
    DP_mode=3
    DIS_mode=21
    no_DRM=1
    ;;    
  "20")
    select_msg="select   20: 4K(40960p60,420), 8bit(297MHz)"
    TV_mode=4096p60
    DP_mode=0
    DIS_mode=27
    no_DRM=1
    ;;
  "21")
    select_msg="select   21: 4K(4096p60,420),10bit(371.25MHz)"
    TV_mode=4096p60
    DP_mode=1
    DIS_mode=27
    no_DRM=1
    ;;
  "22")
    select_msg="select   22: 4K(4096p60,420),12bit(445.5MHz)"
    TV_mode=4096p60
    DP_mode=2
    DIS_mode=27
    no_DRM=1
    ;;
  "23")
    select_msg="select   23: 4K(4096p60,420),16bit(594MHz)"
    TV_mode=4096p60
    DP_mode=3
    DIS_mode=27
    no_DRM=1
    ;;
  "24")
    select_msg="select   24: 4K(40960p30,444), 8bit(297MHz)"
    TV_mode=4096p30
    DP_mode=0
    DIS_mode=25
    no_DRM=1
    ;;
  "25")
    select_msg="select   25: 4K(4096p30,444),10bit(371.25MHz)"
    TV_mode=4096p30
    DP_mode=1
    DIS_mode=25
    no_DRM=1
    ;;
  "26")
    select_msg="select   26: 4K(4096p30,444),12bit(445.5MHz)"
    TV_mode=4096p30
    DP_mode=2
    DIS_mode=25
    no_DRM=1
    ;;
  "27")
    select_msg="select   27: 4K(4096p30,444),16bit(594MHz)"
    TV_mode=4096p30
    DP_mode=3
    DIS_mode=25
    no_DRM=1
    ;;    
  "28")
    select_msg="select   28: 4K(3840p60,444), 8bit(594MHz)"
    TV_mode=3840p60
    DP_mode=0
    format_4K_444=1
    DIS_mode=23
    no_DRM=1
    ;;
  "29")
    select_msg="select   29: 4K(4096p60,444), 8bit(594MHz)"
    TV_mode=4096p60
    DP_mode=0
    format_4K_444=1
    DIS_mode=27
    no_DRM=1
    ;;
  # ---- P type ----    
  "100")
    select_msg="select   10O:  576p, 8bit(27MHz)"
    TV_mode=576p
    DP_mode=0
    DIS_mode=7
    no_DRM=1
    ;;
  "101")
    select_msg="select   101:  576p,10bit(33.75MHz)"
    TV_mode=576p
    DP_mode=1
    DIS_mode=7
    no_DRM=1
    ;;
  "102")
    select_msg="select   102:  576p,12bit(40.5MHz)"
    TV_mode=576p
    DP_mode=2
    DIS_mode=7
    no_DRM=1
    ;;
  "103")
    select_msg="select   103:  576p,16bit(54MHz)"
    TV_mode=576p
    DP_mode=3
    DIS_mode=7
    no_DRM=1
    ;;
  "104")
    select_msg="select   104:  720p50, 8bit(74.25MHz)"
    TV_mode=720p50
    DP_mode=0
    DIS_mode=9
    no_DRM=1
    ;;
  "105")
    select_msg="select   105:  720p50,10bit(92.8125MHz)"
    TV_mode=720p50
    DP_mode=1
    DIS_mode=9
    no_DRM=1
    ;;
  "106")
    select_msg="select   106:  720p50,12bit(111.375MHz)"
    TV_mode=720p50
    DP_mode=2
    DIS_mode=9
    no_DRM=1
    ;;
  "107")
    select_msg="select   107:  720p50,16bit(148.5MHz)"
    TV_mode=720p50
    DP_mode=3
    DIS_mode=9
    no_DRM=1
    ;;
  "108")
    select_msg="select   108: 1080p50, 8bit(148.5MHz)"
    TV_mode=1080p50
    DP_mode=0
    DIS_mode=16
    no_DRM=1
    ;;
  "109")
    select_msg="select   109: 1080p50,10bit(185.625MHz)"
    TV_mode=1080p50
    DP_mode=1
    DIS_mode=16
    no_DRM=1
    ;;
  "110")
    select_msg="select   110: 1080p50,12bit(222.75MHz)"
    TV_mode=1080p50
    DP_mode=2
    DIS_mode=16
    no_DRM=1
    ;;
  "111")
    select_msg="select   111: 1080p50,16bit(297MHz)"
    TV_mode=1080p50
    DP_mode=3
    DIS_mode=16
    no_DRM=1
    ;;
  "112")
    select_msg="select   112: 4K(3840p50,420), 8bit(297MHz)"
    TV_mode=3840p50
    DP_mode=0
    DIS_mode=22
    no_DRM=1
    ;;
  "113")
    select_msg="select   113: 4K(3840p50,420),10bit(371.25MHz)"
    TV_mode=3840p50
    DP_mode=1
    DIS_mode=22
    no_DRM=1
    ;;
  "114")
    select_msg="select   114: 4K(3840p50,420),12bit(445.5MHz)"
    TV_mode=3840p50
    DP_mode=2
    DIS_mode=22
    no_DRM=1
    ;;
  "115")
    select_msg="select   115: 4K(3840p50,420),16bit(594MHz)"
    TV_mode=3840p50
    DP_mode=3
    DIS_mode=22
    no_DRM=1
    ;;
  "116")
    select_msg="select   116: 4K(3840p25,444), 8bit(297MHz)"
    TV_mode=3840p25
    DP_mode=0
    DIS_mode=20
    no_DRM=1
    ;;
  "117")
    select_msg="select   117: 4K(3840p25,444),10bit(371.25MHz)"
    TV_mode=3840p25
    DP_mode=1
    DIS_mode=20
    no_DRM=1
    ;;
  "118")
    select_msg="select   118: 4K(3840p25,444),12bit(445.5MHz)"
    TV_mode=3840p25
    DP_mode=2
    DIS_mode=20
    no_DRM=1
    ;;
  "119")
    select_msg="select   119: 4K(3840p25,444),16bit(594MHz)"
    TV_mode=3840p25
    DP_mode=3
    DIS_mode=20
    no_DRM=1
    ;;    
  "120")
    select_msg="select   120: 4K(40960p50,420), 8bit(297MHz)"
    TV_mode=4096p50
    DP_mode=0
    DIS_mode=26
    no_DRM=1
    ;;
  "121")
    select_msg="select   121: 4K(4096p50,420),10bit(371.25MHz)"
    TV_mode=4096p50
    DP_mode=1
    DIS_mode=26
    no_DRM=1
    ;;
  "122")
    select_msg="select   122: 4K(4096p50,420),12bit(445.5MHz)"
    TV_mode=4096p50
    DP_mode=2
    DIS_mode=26
    no_DRM=1
    ;;
  "123")
    select_msg="select   123: 4K(4096p50,420),16bit(594MHz)"
    TV_mode=4096p50
    DP_mode=3
    DIS_mode=26
    no_DRM=1
    ;;
  "124")
    select_msg="select   124: 4K(40960p25,444), 8bit(297MHz)"
    TV_mode=4096p25
    DP_mode=0
    DIS_mode=24
    no_DRM=1
    ;;
  "125")
    select_msg="select   125: 4K(4096p25,444),10bit(371.25MHz)"
    TV_mode=4096p25
    DP_mode=1
    DIS_mode=24
    no_DRM=1
    ;;
  "126")
    select_msg="select   126: 4K(4096p25,444),12bit(445.5MHz)"
    TV_mode=4096p25
    DP_mode=2
    DIS_mode=24
    no_DRM=1
    ;;
  "127")
    select_msg="select   127: 4K(4096p25,444),16bit(594MHz)"
    TV_mode=4096p25
    DP_mode=3
    DIS_mode=24
    no_DRM=1
    ;;    
  "128")
    select_msg="select   128: 4K(3840p50,444), 8bit(594MHz)"
    TV_mode=3840p50
    DP_mode=0
    format_4K_444=1
    DIS_mode=22
    no_DRM=1
    ;;
  "129")
    select_msg="select   129: 4K(4096p50,444), 8bit(594MHz)"
    TV_mode=4096p50
    DP_mode=0
    format_4K_444=1
    DIS_mode=26
    no_DRM=1
    ;;
  "212")
    select_msg="select  212: 4K(3840p60,420), with HDRM"
    TV_mode=3840p60
    DP_mode=0
    DIS_mode=23
    no_DRM=0
    ;;
  *)
    choose_ret=0
    Display_TV_selection
    ;;
esac



if [ $1 = "d" ]; then
echo "*********************"
echo $select_msg
fi
}


#------------------------------------------------------
#              function: SubMenu_Set_PHY()
#------------------------------------------------------
SubMenu_Set_PHY() {
Set_PHY_running=1
Display_PHY_setting

while [ "$Set_PHY_running" == "1" ]; do
par1=
par2=
par3=
read -t 5 par1 par2 par3
if [ "$par1" == "q" ]; then
Set_PHY_running=0
elif [ "$par1" == "" ]; then
Chk_Play_again
else
case $par1 in
  "dis")
    $vpotest_bin --hdmi dis_phy $par2
    echo ""
    Display_PHY_setting
    ;;
  "set")
    $vpotest_bin --hdmi set_phy $par2 $par3
    echo ""
    Display_PHY_setting
    ;;
  "pattern")
    if [ "$par2" == "on" ]; then
      $vpotest_bin --hdmi wr_reg 0x28 0xaa
      $vpotest_bin --hdmi wr_reg 0x29 0xaa
      $vpotest_bin --hdmi wr_reg 0x0a 0x62
      echo "turn on test pattern"
    else
      $vpotest_bin --hdmi wr_reg 0x0a 0x02
      echo "turn off test pattern"
    fi
    echo ""
    Display_PHY_setting
    ;;    
  "scramble")
    if [ "$par2" == "on" ]; then
      $vpotest_bin --hdmi set_scramble_mode 1
      echo "scramle force on"
    elif [ "$par2" == "off" ]; then
      $vpotest_bin --hdmi set_scramble_mode 2
      echo "scramle force off"
    else
      $vpotest_bin --hdmi set_scramble_mode 0
      echo "scramle normal"    
    fi 
    echo ""
    Display_PHY_setting
    ;;    
  *)
	  Display_PHY_setting
	  ;;
esac 

fi

done 
}


#------------------------------------------------------
#              function: SubMenu_Choose_TV()
#------------------------------------------------------
SubMenu_Choose_TV() {
change_TV_running=1
Display_TV_selection
while [ "$change_TV_running" == "1" ]; do
#echo " TV loop"
change_TV_running=1
# ----- get PHY_mode from user input-----
PHY_mode=
read -t 5 PHY_mode
#echo " select $PHY_mode"
if [ "$PHY_mode" == "q" ]; then
	change_TV_running=0
else
	if [ "$PHY_mode" == "" ]; then
		Chk_Play_again
	elif [ "$PHY_mode" != "" ]; then
		echo " get input: $PHY_mode"
		Choose_mode 0
		if [ $choose_ret == "1" ]; then
			Video_Stop
			ChangeTV $PHY_mode
			Video_PlayBack
			Choose_mode d
			echo "TV_mode: $TV_mode"
			echo "DP_mode: $DP_mode"
			echo "*********************"
			echo ""
			echo ""
			Display_TV_selection
		fi
	fi

fi
done
}

#------------------------------------------------------
#              function: Color_FMT_selection()
#------------------------------------------------------
Color_FMT_selection() {
echo ""
echo ""
echo "************************"
echo "     Sub menu : choose Color Format"
echo "************************"
echo "enter new color format:"
echo "      O: RGB"
echo "      1: YCbCr422"
echo "      2: YCbCr444"
echo "      q: quit to main menu"
}

#------------------------------------------------------
#              function: SubMenu_Choose_Color_FMT()
#------------------------------------------------------
SubMenu_Choose_Color_FMT() {

change_color_fmt_running=1
Color_FMT_selection
while [ "$change_color_fmt_running" == "1" ]; do

color_fmt=
read -t 5 color_fmt


if [ "$color_fmt" == "q" ]; then
	change_color_fmt_running=0
else
	if [ "$color_fmt" == "" ]; then
		Chk_Play_again
	elif [ "$color_fmt" != "" ]; then
		case $color_fmt in
		"0")
		echo "Set color format as 'RGB'."
		$vpotest_bin --hdmi set_color_format 0 > /dev/null 2>&1
		;;
		"1")
		echo "Set color format as 'YCbCr422'."
		$vpotest_bin --hdmi set_color_format 1 > /dev/null 2>&1
		;;
		"2")
		echo "Set color format as 'YCbCr444'."
		$vpotest_bin --hdmi set_color_format 2 > /dev/null 2>&1	
		;; 
		*)
		;;
		esac
		
		Color_FMT_selection
	fi

fi

done

}

#------------------------------------------------------
#              function: aspect_selection()
#------------------------------------------------------
aspect_selection() {
echo ""
echo ""
echo "************************"
echo "     Sub menu : choose aspect"
echo "************************"
echo "enter new aspect ratio:"
echo "      O: 4:3"
echo "      1: 16:9"
echo "      q: quit to main menu"
}

#------------------------------------------------------
#              function: SubMenu_Choose_Aspect()
#------------------------------------------------------
SubMenu_Choose_Aspect() {

change_aspect_running=1

aspect_selection

while [ "$change_aspect_running" == "1" ]; do

aspect=
read -t 5 aspect


if [ "$aspect" == "q" ]; then
	change_aspect_running=0
else
	if [ "$aspect" == "" ]; then
		Chk_Play_again
	elif [ "$aspect" != "" ]; then
		case $aspect in
		"0")
		echo "Set color format as '4:3'."
		$vpotest_bin -d aspect_ratio  3  > /dev/null 2>&1
		;;
		"1")
		echo "Set color format as '9:16'."
		$vpotest_bin -d aspect_ratio  2 > /dev/null 2>&1
		;;
		*)
		;;
		esac
		
		aspect_selection
	fi

fi

done

}


#------------------------------------------------------
#              function: hdcp_selection()
#------------------------------------------------------
hdcp_selection() {
echo ""
echo ""
echo "************************"
echo "     Sub menu : choose hdcp"
echo "************************"
echo "enter new hdcp:"
echo "      O: set hdcp 1.4 key"
echo "      1: set hccp 2.2 key"
echo "      2: set hccp 2.2 CE key"
echo "      3: set hccp SRAM mode"
echo "      4: set hccp CE mode"
echo "      5: set hccp off"
echo "      6: set hccp on"
echo "      7: get hccp link status"
echo "      8: get hccp version"
echo "      q: quit to main menu"
}



#------------------------------------------------------
#              function: SubMenu_Choose_HDCP()
#------------------------------------------------------
SubMenu_Choose_HDCP()
{

change_hdcp_running=1

hdcp_selection

while [ "$change_hdcp_running" == "1" ]; do

hdcp=
read -t 5 hdcp

if [ "$usb_partion_is_sda" == "1" ];then
	usb_dir="sda"
elif [ "$usb_partion_is_sda1" == "1" ];then
	usb_dir="sda1"
elif [ "$usb_partion_is_sda2" == "1" ];then
	usb_dir="sda2"
elif [ "$usb_partion_is_sda3" == "1" ];then
	usb_dir="sda3"
else
	"Cound not find the usb mounted point!!"
	 return 0
fi


if [ "$hdcp" == "q" ]; then
	change_hdcp_running=0
else
	if [ "$hdcp" == "" ]; then
		Chk_Play_again
	elif [ "$hdcp" != "" ]; then
		case $hdcp in
		"0")
		echo "Set hdcp 1.4 SRAM key."
		$vpotest_bin -c set_hdcp14_key  /mnt/usb/$usb_dir/av_inject/hdcp_sw_key.bin 
		;;
		"1")
		echo "Set hdcp 2.2 SRAM key."
		$vpotest_bin -c set_hdcp22_key  /mnt/usb/$usb_dir/av_inject/hdcp22_sw_key.bin 
		;;
		"2")
		echo "Set hdcp 2.2 CE key."
		$vpotest_bin -c set_hdcp22_ce_key  /mnt/usb/$usb_dir/av_inject/hdcp22_ce_key.bin 
		;;
		"3")
		echo "Set hdcp SRAM mode."
		$vpotest_bin -c set_mem_sel 0
		;;
		"4")
		echo "Set hdcp CE mode."
		$vpotest_bin -c set_mem_sel 1
		;;
		"5")
		echo "Set hdcp off."
		$vpotest_bin -c set_on_off 0
		;;
		"6")
		echo "Set hdcp on."
		$vpotest_bin -c set_on_off 1
		;;
		"7")
		echo "Get hdcp link status."
		$vpotest_bin -c get_link_staus
		;;
		"8")
		echo "Get hdcp version."
		$vpotest_bin -c get_version
		;;
		*)
		;;
		esac
		
		hdcp_selection
	fi

fi

done


}


#------------------------------------------------------
#              function: Display_main_menu()
#------------------------------------------------------
Display_main_menu() {
echo ""
echo ""
echo "************************"
echo "     Main menu          "
echo "************************"
echo "enter your choice:"
echo "      t:  choose new TV mode"
echo "      p:  PHY adjust"
echo "      f:  choose color format"
echo "      a:  choose aspect ratio"
echo "      h:  choose HDCP"
echo ""
echo "      q: quit"
echo " others: display selection"
echo ""
}

#------------------------------------------------------
#              function: Choose_sub_menu()
#------------------------------------------------------
Choose_sub_menu() {
sub_menu_ret=0
case $sub_menu in
  "t")
    sub_menu_ret=1
    ;;
  "T")
    sub_menu_ret=1
    ;;
  "p")
    sub_menu_ret=2
    ;;
  "P")
    sub_menu_ret=2
    ;;
  "f")
    sub_menu_ret=3
    ;;
  "F")
    sub_menu_ret=3
    ;;
  "a")
    sub_menu_ret=4
    ;;
  "A")
    sub_menu_ret=4
    ;;
  "h")
    sub_menu_ret=5
    ;;
  "H")
    sub_menu_ret=5
    ;;
  *)
    sub_menu_ret=0
	  ;;
esac 
}


#------------------------------------------------------
#              function: Chk_Play_again()
#------------------------------------------------------
video_running=0
Chk_Play_again() {

if [ -e /proc/$Video_PlayBack_pid -a /proc/$Video_PlayBack_pid/exe ]; then
video_running=1
else
video_running=0
echo "video play again"
Video_PlayBack
fi

#echo "video_running = $video_running"

}

#------------------------------------------------------
#              function: trap_INT_handle()
#------------------------------------------------------
trap_INT_handle() {
echo "user quit"
Video_Stop

aui_dis_cur

exit 0
}



##############################################################################
#                                   Main
##############################################################################
echo -n "Starting CTS testing: "

if cat /proc/partitions |  grep -i sda3 > /dev/null 2>&1 ; then
	if df -h |  grep -i sda3 ;then
		echo "sda3 has been mounted!!"
	else
		mount -t vfat /dev/sda3 /mnt > /dev/null 2>&1
	fi
	usb_partion_is_sda3=1
elif cat /proc/partitions |  grep -i sda2 > /dev/null 2>&1 ; then
	if df -h |  grep -i sda2 ;then
		echo "sda2 has been mounted!!"
	else
		mount -t vfat /dev/sda2 /mnt > /dev/null 2>&1
	fi
	usb_partion_is_sda2=1
elif cat /proc/partitions |  grep -i sda1 > /dev/null 2>&1 ; then
	if df -h |  grep -i sda1 ;then
		echo "sda1 has been mounted!!"
	else
		mount -t vfat /dev/sda1 /mnt > /dev/null 2>&1
	fi
	usb_partion_is_sda1=1
elif cat /proc/partitions |  grep -i sda > /dev/null 2>&1 ; then
	if df -h |  grep -i sda ;then
		echo "sda has been mounted!!"
	else
		mount -t vfat /dev/sda /mnt > /dev/null 2>&1
	fi
	usb_partion_is_sda=1
else
	echo "No USB partions could be mounted !!"
	exit 0
fi


trap trap_INT_handle INT
#Video_PlayBack

echo ""
echo ""
echo ""
echo "####################################################################"
echo "#####                                                          #####"
echo "#####                                                          #####"
echo "#####            S3922 CTS test                                #####"
echo "#####                                                          #####"
echo "#####                                                          #####"
echo "####################################################################"


TV_mode=1080p60
DP_mode=0

# -----------------
#   main menu
# -----------------
Display_main_menu

while [ 1 ]; do
sub_menu=
read -t 5 sub_menu
# -- get 'q'?
if [ "$sub_menu" == "q" ]; then
trap_INT_handle
fi
#
if [ "$sub_menu" == "" ]; then
Chk_Play_again
else
echo " get choice: $sub_menu"
Choose_sub_menu
case $sub_menu_ret in
  "1")    
    SubMenu_Choose_TV
    Display_main_menu
    ;;
  "2")
    SubMenu_Set_PHY
    Display_main_menu
    ;;
  "3")
    SubMenu_Choose_Color_FMT
    Display_main_menu
    ;;
  "4")
    SubMenu_Choose_Aspect
    Display_main_menu
    ;;
  "5")
    SubMenu_Choose_HDCP
    Display_main_menu
    ;;
   *)
    sub_menu_ret=0
    Display_main_menu
    ;;
esac 
fi
done

