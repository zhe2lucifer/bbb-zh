#================================================================================================================
#
#  test_PHY.sh
#
#    This script is used to test HDMI PHY output.
#
#    Usage:
#        ./test_PHY.sh
#
#
#    Description:
#        This script will 
#            (i) call application "vpotest" to change TV system, then
#            (ii) call application "videotest" to playback stream
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
Video_PlayBack_By_EDID_pid=0
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
Video_PlayBack_By_EDID () {


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

play_video_file=/mnt/usb/$usb_dir/av_inject/Sony_4K_Surfing.mp4
$videotest_bin -k $play_video_file 0 h265 3840 2160 60 aac 48000  > /dev/null 2>&1 &

Video_PlayBack_By_EDID_pid=$!

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
$videotest_bin -v   $play_video_file  0  h265 3840 2160 60 aac 48000  > /dev/null 2>&1 &

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
  if [ "$Video_PlayBack_By_EDID_pid" != "0" ]; then
   kill -2 $Video_PlayBack_By_EDID_pid > /dev/null
   Video_PlayBack_By_EDID_pid=0
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
echo "      O:  480p, 8bit(27MHz)"
echo "      1:  480p,10bit(33.75MHz)"
echo "      2:  480p,12bit(40.5MHz)"
echo "      3:  480p,16bit(54MHz)"
echo "      4:  720p, 8bit(74.25MHz)"
echo "      5:  720p,10bit(92.8125MHz)"
echo "      6:  720p,12bit(111.375MHz)"
echo "      7:  720p,16bit(148.5MHz)"
echo "      8: 1080p, 8bit(148.5MHz)"
echo "      9: 1080p,10bit(185.625MHz)"
echo "     10: 1080p,12bit(222.75MHz)"
echo "     11: 1080p,16bit(297MHz)"
echo "     12: 4K(420), 8bit(297MHz)"
echo "     13: 4K(420),10bit(371.25MHz)"
echo "     14: 4K(420),12bit(445.5MHz)"
echo "     15: 4K(420),16bit(594MHz)"
echo "     16: 4K(444), 8bit(594MHz)"
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

if [ $PHY_mode = "0" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   O:  480p, 8bit(27MHz)"
fi
TV_mode=480p
DP_mode=0
DIS_mode=8
no_DRM=1

elif [ $PHY_mode = "1" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    1:  480p,10bit(33.75MHz)"
fi
TV_mode=480p
DP_mode=1
DIS_mode=8
no_DRM=1

elif [ $PHY_mode = "2" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    2:  480p,12bit(40.5MHz)"
fi
TV_mode=480p
DP_mode=2
DIS_mode=8
no_DRM=1

elif [ $PHY_mode = "3" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    3:  480p,16bit(54MHz)"
fi
TV_mode=480p
DP_mode=3
DIS_mode=8
no_DRM=1

elif [ $PHY_mode = "4" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    4:  720p, 8bit(74.25MHz)"
fi
TV_mode=720p60
DP_mode=0
DIS_mode=10
no_DRM=1

elif [ $PHY_mode = "5" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    5:  720p,10bit(92.8125MHz)"

fi
TV_mode=720p60
DP_mode=1
DIS_mode=10
no_DRM=1

elif [ $PHY_mode = "6" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    6:  720p,12bit(111.375MHz)"
fi
TV_mode=720p60
DP_mode=2
DIS_mode=10
no_DRM=1

elif [ $PHY_mode = "7" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    7:  720p,16bit(148.5MHz)"
fi
TV_mode=720p60
DP_mode=3
DIS_mode=10
no_DRM=1

elif [ $PHY_mode = "8" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    8: 1080p, 8bit(148.5MHz)"
fi
TV_mode=1080p60
DP_mode=0
DIS_mode=17
no_DRM=1

elif [ $PHY_mode = "9" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select    9: 1080p,10bit(185.625MHz)"
fi
TV_mode=1080p60
DP_mode=1
DIS_mode=17
no_DRM=1

elif [ $PHY_mode = "10" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   10: 1080p,12bit(222.75MHz)"
fi
TV_mode=1080p60
DP_mode=2
DIS_mode=17
no_DRM=1

elif [ $PHY_mode = "11" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   11: 1080p,16bit(297MHz)"
fi
TV_mode=1080p60
DP_mode=3
DIS_mode=17
no_DRM=1

elif [ $PHY_mode = "12" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   12: 4K(420), 8bit(297MHz)"
fi
TV_mode=3840p60
DP_mode=0
DIS_mode=23
no_DRM=1

elif [ $PHY_mode = "13" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   13: 4K(420),10bit(371.25MHz)"
fi
TV_mode=3840p60
DP_mode=1
DIS_mode=23
no_DRM=1

elif [ $PHY_mode = "14" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   14: 4K(420),12bit(445.5MHz)"
fi
TV_mode=3840p60
DP_mode=2
DIS_mode=23
no_DRM=1

elif [ $PHY_mode = "15" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   15: 4K(420),16bit(594MHz)"
fi
TV_mode=3840p60
DP_mode=3
DIS_mode=23
no_DRM=1

elif [ $PHY_mode = "16" ]; then

if [ $1 = "d" ]; then
echo "*********************"
echo "select   16: 4K(444), 8bit(594MHz)"
fi
TV_mode=3840p60
DP_mode=0
format_4K_444=1   
DIS_mode=23
no_DRM=1

else

choose_ret=0
Display_TV_selection

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
    $vpotest_bin --hdmi dis_phy $par2 > /dev/null 2>&1
    echo ""
    Display_PHY_setting
    ;;
  "set")
    $vpotest_bin --hdmi set_phy $par2 $par3 > /dev/null 2>&1
    echo ""
    Display_PHY_setting
    ;;
  "pattern")
    if [ "$par2" == "on" ]; then
      $vpotest_bin --hdmi wr_reg 0x28 0xaa > /dev/null 2>&1
      $vpotest_bin --hdmi wr_reg 0x29 0xaa > /dev/null 2>&1
      $vpotest_bin --hdmi wr_reg 0x0a 0x62 > /dev/null 2>&1
      echo "turn on test pattern"
    else
      $vpotest_bin --hdmi wr_reg 0x0a 0x02 > /dev/null 2>&1
      echo "turn off test pattern"
    fi
    echo ""
    Display_PHY_setting
    ;;    
  "scramble")
    if [ "$par2" == "on" ]; then
      $vpotest_bin --hdmi set_scramble_mode 1 > /dev/null 2>&1
      echo "scramle force on"
    elif [ "$par2" == "off" ]; then
      $vpotest_bin --hdmi set_scramble_mode 2 > /dev/null 2>&1
      echo "scramle force off"
    else
      $vpotest_bin --hdmi set_scramble_mode 0 > /dev/null 2>&1
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
#              function: SubMenu_Loop_TV()
#------------------------------------------------------
SubMenu_Loop_TV() {
echo "==============================="
echo "==  TV mode change loop test =="
echo "==============================="
loop_TV_running=1

lp_TV_mode=0
PHY_mode=$lp_TV_mode
Choose_mode d
Video_Stop
ChangeTV $PHY_mode
Video_PlayBack

lp_TV_mode=1

while [ "$loop_TV_running" == "1" ]; do

quit_loop=
read -t 8 quit_loop

if [ "$quit_loop" == "q" ]; then
	loop_TV_running=0
else
	PHY_mode=$lp_TV_mode
	Choose_mode d
	Video_Stop
	ChangeTV $PHY_mode
	Video_PlayBack
	lp_TV_mode=$(( $lp_TV_mode + 1 ))
	if [ "$lp_TV_mode" == "16" ];  then
		lp_TV_mode=0
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
echo "      l:  loop TV mode change automatically"
echo "      d on: debug print on"
echo "      d off: debug print off"
echo "      f:  choose color format"
echo "      a:  choose aspect ratio"
echo "      e on:  enable automatically playing video according to EDID"
echo "      e off: disable automatically playing video according to EDID"
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
  "l")
    sub_menu_ret=3
    ;;
  "L")
    sub_menu_ret=3
    ;;
  "d")
    sub_menu_ret=4
    ;;
  "D")
    sub_menu_ret=4
    ;;
  "e")
    sub_menu_ret=5
    ;;
  "f")
    sub_menu_ret=6
    ;;
  "F")
    sub_menu_ret=6
    ;;
  "a")
    sub_menu_ret=7
    ;;
  "A")
    sub_menu_ret=7
    ;;
  *)
    sub_menu_ret=0
	  ;;
esac 
}


#------------------------------------------------------
#              function: Chk_Play_again()
#------------------------------------------------------
Chk_Play_again() {

if [ "$Video_PlayBack_By_EDID_pid" == "0" ]; then

if [ -e /proc/$Video_PlayBack_pid -a /proc/$Video_PlayBack_pid/exe ]; then
video_running=1
else
video_running=0
echo "video play again"
Video_PlayBack
fi

fi
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
echo -n "Starting HDMI PHY measurement: "

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
echo "#####            S3922 PHY test                                #####"
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
param=
read -t 5 sub_menu param
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
    SubMenu_Loop_TV
    Display_main_menu
    ;;
  "4")
    if [ "$param" == "on" ]; then
    echo 8 > /proc/sys/kernel/printk
    elif [ "$param" == "off" ]; then
    echo 7 > /proc/sys/kernel/printk
    fi
    Display_main_menu
    ;;
   "5")
   if [ "$param" == "on" ]; then
    Video_Stop
    Video_PlayBack_By_EDID
   elif [ "$param" == "off" ]; then
    Video_Stop
   fi
   Display_main_menu
    ;;
   "6")
    SubMenu_Choose_Color_FMT
    Display_main_menu
    ;;
   "7")
    SubMenu_Choose_Aspect
    Display_main_menu
    ;;
   *)
    sub_menu_ret=0
    Display_main_menu
	  ;;
esac 
fi
done

