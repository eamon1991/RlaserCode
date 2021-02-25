using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Windows.Forms.DataVisualization.Charting;
using System.Reflection;
using System.IO;
using CCWin;
using System.Runtime.InteropServices;
using ComPortUser;


namespace upper_soft
{
    /// <summary>
    /// 读取参数结构体
    /// 成员：红光电流、PD1电压、PD2电压、电流报警值、最大电流值、温度报警值、报警屏蔽码
    /// </summary>
    public struct struct_para_read
    {
        public double Red_Current_Value;
        public double PD1_Value;
        public double PD2_Value;
        public double Warn_Current_Value;
        public double Max_Current_Value;
        public byte Warn_Temp_Value;
        public UInt16 Screen_Code;
    }

    /// <summary>
    /// 解锁信息结构体
    /// </summary>
    public struct struct_lock
    {
        public byte LockFlag;
        public byte LockType;
        public byte LockNum;
        public int LockYear;
        public int LockMonth;
        public int LockDay;
        public int LockHour;
        public int LockMin;
        public string LockTime;
    }

    /// <summary>
    /// 设备状态信息结构体：包括参数结构体、自锁信息结构体
    /// </summary>
    public struct struct_Unit_Info
    {
        /////////////////////////////////
        public string UnitType;                 //设备型号
        public string UnitSN;                   //设备序列号
        public string UnitSV;                   //设备软件版本号
        /////////////////////////////////
        public byte Power;                      //功率值
        public byte DriverModeNum;              //驱动板个数
        public byte[] TempValue;                //温度值0-4
        public double[] CurrentValue;           //电流值0-6
        /////////////////////////////////
        public struct_para_read Para_Read;      //参数读取值
        /////////////////////////////////
        public struct_lock Lock_Info;           //自锁信息
        /////////////////////////////////
        public int unit_year;                   //设备时间-年
        public int unit_month;                  //设备时间-月
        public int unit_day;                    //设备时间-日
        public int unit_hour;                   //设备时间-时
        public int unit_min;                    //设备时间-分
        public int unit_sec;                    //设备时间-秒
        public string unit_time;                //设备时间 （年-月-日 时：分：秒）
        //////////////////////////////////
        public byte LogNum;                     //登录次数
        //////////////////////////////////
        public bool alarmFlag;                  //报警标志
        public byte alarmCode;                  //报警代码
    }

    public partial class Form1 : Form
    {
        #region 参数区
        public comPort com;
        public struct_Unit_Info unit_info;
        //txt文本操作参数
        public string AlarmFile_name;
        public string AlarmFile_path;
        public string AlarmFile_base_path = "C:\\record";
        public bool AlarmFile_isLoad = false;
        public bool AlarmFile_isOpen = false;
        //ini文件路径参数
        string ini_path = Application.StartupPath + @"\config.ini";
        #endregion

        #region 窗口方法
        public Form1()
        {
            InitializeComponent();
        }
        #endregion

        public void file_init()
        {
            //错误记录文件
            if (Directory.Exists(AlarmFile_base_path) == false)//如果不存在就创建file文件夹
            {
                Directory.CreateDirectory(AlarmFile_base_path);
            }

            AlarmFile_name = DateTime.Now.Year.ToString("0000") + DateTime.Now.Month.ToString("00") + DateTime.Now.Day.ToString("00") + ".TXT";
            AlarmFile_path = AlarmFile_base_path + "\\" + AlarmFile_name;
            //ini配置文件
            if (File.Exists(ini_path) == false)
            {
                WriteIniData("CIPHER CODE", "time_used", "0", ini_path);
            }
           // label26.Text = "已解密" + ReadIniData("CIPHER CODE", "time_used", "255", ini_path) + "次";

        }

        #region 窗口导入-初始化
        private void Form1_Load(object sender, EventArgs e)
        {
            file_init();
            //隐藏 监控、参数设置、历史记录、网络监控的界面
            skinTabPage2.Parent = skinTabPage4.Parent = skinTabPage3.Parent = skinTabPage5.Parent = null;
            //com实例化
            com = new comPort();
            //设备信息数据实例化
            unit_info = new struct_Unit_Info();
            unit_info.CurrentValue = new double[6];
            unit_info.TempValue = new byte[4];
            unit_info.alarmFlag = false;//初始化报警标志
            //chart初始化
            DateTime t = DateTime.Now;//更新X轴坐标时间
            chart1.ChartAreas[0].AxisX.CustomLabels.Clear();
            for (int i = 0; i <= 20; i++)
            {
                if (i % 4 == 0)
                {
                    CustomLabel label1 = new CustomLabel();
                    label1.ForeColor = Color.Black;
                    label1.Text = t.ToLongTimeString();
                    label1.ToPosition = i;
                    chart1.ChartAreas[0].AxisX.CustomLabels.Add(label1);
                    t = t.AddSeconds(1);
                }
            }
            //用于界面更新的委托初始化
            DisRefresh = new interfaceRefresh(UnitStatusDis);


            //设置界面锁住
            ParaSetLock();

            //初始化自锁时间，避免格式出错
            unit_info.Lock_Info.LockYear = 2019;
            unit_info.Lock_Info.LockMonth = 10;
            unit_info.Lock_Info.LockDay = 1;
            unit_info.Lock_Info.LockHour = 0;
            unit_info.Lock_Info.LockMin = 0;
        }
        #endregion

        #region 软件登录界面-串口本地监控
        /// <summary>
        /// 获取串口号：登录操作
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void CbxCom_Click(object sender, EventArgs e)
        {
            com.cbxGetName(cbxCom);
        }
        private void BtnLogIn_Click(object sender, EventArgs e)
        {
            if (com.SetPortProperty(cbxCom.Text))   //若串口设置成功，则进行登录
            {
                //显示监控、参数设置、历史记录、网络监控界面，隐藏登录界面
                skinTabPage1.Parent = null;
                /*skinTabPage5.Parent = */skinTabPage4.Parent = skinTabPage3.Parent = skinTabPage2.Parent = skinTabControl1;
                toolStripStatusLabel1.ForeColor = Color.Green;
                toolStripStatusLabel1.Text = "已连接" + cbxCom.Text.ToString() + "!";
                thread_init();  //创建一个线程：用于提取设备返回的数据
                timer1.Start(); //开启定时刷新界面
            }
        }
        #endregion

        #region 窗口闪屏
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams CP = base.CreateParams;
                CP.ExStyle |= 0x02000000;
                return CP;
            }
        }
        #endregion

        #region 控制内控出光
        /// <summary>
        /// 开启内控出光
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void BtnLaserOn_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };
            cmd[10] = Convert.ToByte(cbxPower.Text);
            cmd[11] = Convert.ToByte(cbxFre.Text);
            cmd[12] = Convert.ToByte(cbxDuty.Text);
            com.RS232_Send(cmd);
        }
        /// <summary>
        /// 关闭内控出光
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void BtnLaserOff_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x01, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }
        #endregion

        #region 打开模块报警功能
        private void btnPD1On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x01, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void btnPD2On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x02, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnAllAlarmOn_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0F, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }
        private void BtnQbhOpen1_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x03, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnQbhOpen_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x03, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }
        private void BtnWatterOn_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0E, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp1On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x04, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp2On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x05, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp3On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x06, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp4On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent1On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x08, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent2On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x09, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent3On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0A, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent4On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0B, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent5On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0C, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent6On_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0D, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }
        #endregion

        #region 关闭模块报警功能
        private void btnPD1Close_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x01, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void btnPD2Close_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x02, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnAllAlarmOff_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0F, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }
        private void BtnQbhClose_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x03, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }
        private void BtnQbhClose1_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x03, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }
        private void BtnWatterOff_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0E, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp1Close_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x04, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp2Close_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x05, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp3Close_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x06, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnTemp4Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent1Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x08, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent2Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x09, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent3Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0A, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent4Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0B, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent5Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0C, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnCurrent6Off_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x0D, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }
        #endregion

        #region 设置参数、SN号、设备时间
        private void BtnRedCurrentSet_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x02, 0x00, 0x00 };
            cmd[9] = Convert.ToByte((float.Parse(cbxRedCurrentSetValue.Text)) * 1.175);
            com.RS232_Send(cmd);
        }

        private void BtnPD1Set_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x00, 0x00 };
            cmd[9] = Convert.ToByte((float.Parse(cbxPD1SetValue.Text)) * 10);
            com.RS232_Send(cmd);
        }

        private void BtnPD2Set_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x04, 0x00, 0x00 };
            cmd[9] = Convert.ToByte((float.Parse(cbxPD2ValueSet.Text)) * 10);
            com.RS232_Send(cmd);
        }

        private void BtnTempSet_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x05, 0x00, 0x00 };
            cmd[9] = Convert.ToByte(cbxTempSetValue.Text);
            com.RS232_Send(cmd);
        }

        private void BtnCurrentSet_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x06, 0x00, 0x00,0x00 };
            cmd[9] = (Byte)((Convert.ToUInt16((float.Parse(cbxCurrentSetvalue.Text)) * 10) >> 8) & 0xFF);
            cmd[10] = (Byte)((Convert.ToUInt16((float.Parse(cbxCurrentSetvalue.Text)) * 10)) & 0xFF);

            //eamon 2021.2.22
            if (Convert.ToUInt16((float.Parse(cbxCurrentSetvalue.Text))) > 36) {
                MessageBox.Show("超出参数范围0-36,请重新设置！");
            }
            else
            {
                com.RS232_Send(cmd);
            }
        }

        private void BtnMaxCurrentSet_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x07, 0x00, 0x00, 0x00 };
            cmd[9] = (Byte)((Convert.ToUInt16((float.Parse(cbxMaxCurrentSetValue.Text)) * 10) >> 8) & 0xFF);
            cmd[10] = (Byte)((Convert.ToUInt16((float.Parse(cbxMaxCurrentSetValue.Text)) * 10)) & 0xFF);


            //eamon 2021.2.22
            if (Convert.ToUInt16((float.Parse(cbxMaxCurrentSetValue.Text))) > 36)
            {
                MessageBox.Show("超出参数范围0-36,请重新设置！");
            }
            else
            {
                com.RS232_Send(cmd);
            }
        }

        private void BtnSetSN_Click(object sender, EventArgs e)
        {
            try
            {
                byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                cmd[9] = (byte)Convert.ToChar(tbxSN.Text.Substring(0, 1));
                cmd[10] = Convert.ToByte(tbxSN.Text.Substring(1, 2));

                cmd[11] = (byte)((Convert.ToInt16(tbxSN.Text.Substring(3, 3))) >> 8);
                cmd[12] = (byte)((Convert.ToInt16(tbxSN.Text.Substring(3, 3))));

                cmd[13] = Convert.ToByte(tbxSN.Text.Substring(6, 2));
                cmd[14] = Convert.ToByte(tbxSN.Text.Substring(8, 2));
                if (tbxSN.Text == "A000000000") MessageBox.Show("不支持此序列号，请重新输入!");
                else com.RS232_Send(cmd);
            }
            catch
            {
                MessageBox.Show("输入序列号错误！");
            }

        }

        private void BtnParaLockSet_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x07, 0x00 };
            com.RS232_Send(cmd);
        }

        private void BtnGetLocalTime_Click(object sender, EventArgs e)
        {
            DateTime t = DateTime.Now;
            tbxYear.Text = t.Year.ToString("0000");
            tbxMonth.Text = t.Month.ToString("00");
            tbxDay.Text = t.Day.ToString("00");
            tbxHour.Text = t.Hour.ToString("00");
            tbxMin.Text = t.Minute.ToString("00");
            tbxSec.Text = t.Second.ToString("00");
        }

        private void BtnUnityTimeSet_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            cmd[9] = (byte)(Convert.ToInt16(tbxYear.Text) >> 8);
            cmd[10] = (byte)(Convert.ToInt16(tbxYear.Text));
            cmd[11] = Convert.ToByte(tbxMonth.Text);
            cmd[12] = Convert.ToByte(tbxDay.Text);
            cmd[13] = Convert.ToByte(tbxHour.Text);
            cmd[14] = Convert.ToByte(tbxMin.Text);
            cmd[15] = Convert.ToByte(tbxSec.Text);
            com.RS232_Send(cmd);
        }


        #endregion

        /// <summary>
        /// 串口接收数据线程
        /// </summary>
        public System.Threading.Thread dat_para;
        public void thread_init()
        {
            dat_para = new System.Threading.Thread(get_device_dat);
            dat_para.IsBackground = true;
            dat_para.Start();
        }

        /// <summary>
        /// 串口通信：提取数据
        /// </summary>
        public byte err_code;
        public byte year;
        public byte month;
        public byte day;
        public byte hour;
        public byte min;
        public byte sec;
        public void get_device_dat()
        {
            while (true)
            {
                if (com.receive_flag == true)
                {
                    //复制数据段
                    byte[] dat = new byte[255];
                    dat[0] = (byte)(com.frmFmt_1.flag);
                    dat[1] = (byte)(com.frmFmt_1.SrcAddr >> 8);
                    dat[2] = (byte)(com.frmFmt_1.SrcAddr);
                    dat[3] = (byte)(com.frmFmt_1.DestAddr >> 8);
                    dat[4] = (byte)(com.frmFmt_1.DestAddr);
                    dat[5] = (byte)(com.frmFmt_1.Type);
                    dat[6] = (byte)(com.frmFmt_1.Length);
                    try
                    {
                        for (int i = 0; i < com.frmFmt_1.Length; i++) dat[i + 7] = (byte)(com.frmFmt_1.buf[i]);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                    //end
                    //数据段CRC计算并与数据段CRC校验码比对校验
                    if (com.frmFmt_1.verify == com.get_crc(dat, (byte)(com.frmFmt_1.Length + 7)))
                    {
                        //提取命令字：并解析操作
                        switch ((com.frmFmt_1.buf[0] << 8) | (com.frmFmt_1.buf[1]))
                        {
                            case 0x0106:    //提取设备信息命令
                                //参数数据提取
                                unit_info.Para_Read.Red_Current_Value = com.frmFmt_1.buf[2];
                                unit_info.Para_Read.PD1_Value = com.frmFmt_1.buf[3] / 10.0;
                                unit_info.Para_Read.PD2_Value = com.frmFmt_1.buf[4] / 10.0;
                                unit_info.Para_Read.Warn_Temp_Value = com.frmFmt_1.buf[5];
                                unit_info.Para_Read.Warn_Current_Value = ((UInt16)(com.frmFmt_1.buf[44] << 8) | (com.frmFmt_1.buf[6])) / 10.0;
                                unit_info.Para_Read.Max_Current_Value = ((UInt16)(com.frmFmt_1.buf[45] << 8) | (com.frmFmt_1.buf[7])) / 10.0;
                                //报警屏蔽码提取
                                unit_info.Para_Read.Screen_Code = (UInt16)((com.frmFmt_1.buf[8] << 8) | com.frmFmt_1.buf[9]);
                                //自锁信息提取
                                unit_info.Lock_Info.LockFlag = com.frmFmt_1.buf[10];
                                unit_info.Lock_Info.LockType = com.frmFmt_1.buf[11];
                                unit_info.Lock_Info.LockNum = com.frmFmt_1.buf[12];
                                unit_info.Lock_Info.LockYear = (int)((com.frmFmt_1.buf[13] << 8) | com.frmFmt_1.buf[14]);
                                unit_info.Lock_Info.LockMonth = com.frmFmt_1.buf[15];
                                unit_info.Lock_Info.LockDay = com.frmFmt_1.buf[16];
                                unit_info.Lock_Info.LockHour = com.frmFmt_1.buf[17];
                                unit_info.Lock_Info.LockMin = com.frmFmt_1.buf[18];
                                unit_info.Lock_Info.LockTime = unit_info.Lock_Info.LockYear.ToString("0000") + "-" +
                                    unit_info.Lock_Info.LockMonth.ToString("00") + "-" +
                                    unit_info.Lock_Info.LockDay.ToString("00") + " " +
                                    unit_info.Lock_Info.LockHour.ToString("00") + ":" +
                                    unit_info.Lock_Info.LockMin.ToString("00") + ":" + "00";
                                //设备内部时间提取
                                unit_info.unit_year = (int)((com.frmFmt_1.buf[19] << 8) | com.frmFmt_1.buf[20]);
                                unit_info.unit_month = com.frmFmt_1.buf[21];
                                unit_info.unit_day = com.frmFmt_1.buf[22];
                                unit_info.unit_hour = com.frmFmt_1.buf[23];
                                unit_info.unit_min = com.frmFmt_1.buf[24];
                                unit_info.unit_sec = com.frmFmt_1.buf[25];
                                unit_info.unit_time = unit_info.unit_year.ToString("0000") + "-" +
                                    unit_info.unit_month.ToString("00") + "-" +
                                    unit_info.unit_day.ToString("00") + " " +
                                    unit_info.unit_hour.ToString("00") + ":" +
                                    unit_info.unit_min.ToString("00") + ":" +
                                    unit_info.unit_sec.ToString("00");
                                //功率信息提取
                                unit_info.Power = com.frmFmt_1.buf[26];
                                //4路温度信息提取
                                unit_info.TempValue[0] = com.frmFmt_1.buf[27];
                                unit_info.TempValue[1] = com.frmFmt_1.buf[28];
                                unit_info.TempValue[2] = com.frmFmt_1.buf[29];
                                unit_info.TempValue[3] = com.frmFmt_1.buf[30];
                                //驱动板个数提取
                                unit_info.DriverModeNum = com.frmFmt_1.buf[31];
                                //序列号提取
                                unit_info.UnitSN = ((char)com.frmFmt_1.buf[32]).ToString() +
                                    com.frmFmt_1.buf[33].ToString("00") +   //型号
                                    ((com.frmFmt_1.buf[34] << 8) | com.frmFmt_1.buf[35]).ToString("000") +   //设备编号
                                com.frmFmt_1.buf[36].ToString("00") +   //99-年份后两位
                                    com.frmFmt_1.buf[37].ToString("00");  //99-月份
                                    
                                //登录信息及代码版本信息提取
                                unit_info.LogNum = com.frmFmt_1.buf[38];
                                unit_info.UnitSV = "V" + (((com.frmFmt_1.buf[39] << 8) | com.frmFmt_1.buf[40]) / 100.0).ToString("0.00");
                                //电流信息提取
                                unit_info.CurrentValue[0] = ((UInt16)(com.frmFmt_1.buf[46] << 8) | (com.frmFmt_1.buf[47])) / 10.0;
                                unit_info.CurrentValue[1] = ((UInt16)(com.frmFmt_1.buf[48] << 8) | (com.frmFmt_1.buf[49])) / 10.0;
                                unit_info.CurrentValue[2] = ((UInt16)(com.frmFmt_1.buf[50] << 8) | (com.frmFmt_1.buf[51])) / 10.0;
                                unit_info.CurrentValue[3] = ((UInt16)(com.frmFmt_1.buf[52] << 8) | (com.frmFmt_1.buf[53])) / 10.0;
                                unit_info.CurrentValue[4] = ((UInt16)(com.frmFmt_1.buf[54] << 8) | (com.frmFmt_1.buf[55])) / 10.0;
                                unit_info.CurrentValue[5] = ((UInt16)(com.frmFmt_1.buf[56] << 8) | (com.frmFmt_1.buf[57])) / 10.0;
                                if (unit_info.CurrentValue[0] < 0.6) unit_info.CurrentValue[0] = 0;
                                if (unit_info.CurrentValue[1] < 0.6) unit_info.CurrentValue[1] = 0;
                                if (unit_info.CurrentValue[2] < 0.6) unit_info.CurrentValue[2] = 0;
                                if (unit_info.CurrentValue[3] < 0.6) unit_info.CurrentValue[3] = 0;
                                if (unit_info.CurrentValue[4] < 0.6) unit_info.CurrentValue[4] = 0;
                                if (unit_info.CurrentValue[5] < 0.6) unit_info.CurrentValue[5] = 0;
                                break;

                            case 0x0104:    //提取设备报警命令 
                                //提取报警代码
                                unit_info.alarmCode = com.frmFmt_1.buf[2];
                                //置位报警标志
                                unit_info.alarmFlag = true;
                                break;

                            default: break;

                            case 0x0107:
                                err_code = com.frmFmt_1.buf[2];
                                is_alarm_History = true; //get_err_record = true; //借用 get_err_record
                                year = com.frmFmt_1.buf[3];
                                month = com.frmFmt_1.buf[4];
                                day = com.frmFmt_1.buf[5];
                                hour = com.frmFmt_1.buf[6];
                                break;
                        }
                    }
                    com.receive_flag = false;
                }
            }
        }

        /// <summary>
        /// 设备状态显示，包括：型号、序列号、版本号、功率、解锁、温度、报警功能状态、电流、温度计、折线图、参数、设备时间
        /// </summary>
        public Label[] alarmScreenLabel = new Label[14];
        public double[] temp1 = new double[200];
        public double[] temp2 = new double[200];
        public double[] temp3 = new double[200];
        public double[] temp4 = new double[200];
        public double[] ix1 = new double[200];
        public double[] ix2 = new double[200];
        public double[] ix3 = new double[200];
        public double[] ix4 = new double[200];
        public double[] ix5 = new double[200];
        public double[] ix6 = new double[200];
        
        public void UnitStatusDis()
        {
            //================ 0 : Unit Type-设备型号============================
            labelUnitType.Text = "单模激光器300W";
            if (labelSN.Text.Length > 3)
            {
                if ((labelSN.Text.Substring(0, 3)).CompareTo("S02") == 0) labelUnitType.Text = "单模连续激光器200W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S01") == 0) labelUnitType.Text = "单模连续激光器100W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S03") == 0) labelUnitType.Text = "单模连续激光器300W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S00") == 0) labelUnitType.Text = "单模连续激光器350W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S05") == 0) labelUnitType.Text = "单模连续激光器500W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S08") == 0) labelUnitType.Text = "单模连续激光器800W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S10") == 0) labelUnitType.Text = "单模连续激光器1000W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S15") == 0) labelUnitType.Text = "单模连续激光器1500W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S20") == 0) labelUnitType.Text = "单模连续激光器2000W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S30") == 0) labelUnitType.Text = "单模连续激光器3000W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S40") == 0) labelUnitType.Text = "单模连续激光器4000W";
                else if ((labelSN.Text.Substring(0, 3)).CompareTo("S50") == 0) labelUnitType.Text = "单模连续激光器5000W";
            }
            //================ 1 : Unit SN-设备序列号============================
            labelSN.Text = unit_info.UnitSN;
            //================ 2 : Soft Version-下位机软件版本号==================
            labelVN.Text = unit_info.UnitSV;
            //================ 3 : Power-当前功率================================
            labelPower.Text = unit_info.Power.ToString("000") + "%";
            //================ 4 : Lock Info解锁信息=============================
            /*if (unit_info.Lock_Info.LockNum >= 1)*/ skinLabel22.Text = "已解密" + (unit_info.Lock_Info.LockNum).ToString() + "次";
            //else skinLabel22.Text = "已解密0次";
#if true
            if ((unit_info.Lock_Info.LockType < 6) /*&& (unit_info.Lock_Info.LockNum >= 1)*/)
            {
                int LockTimeAll = 30 * (unit_info.Lock_Info.LockType + 1);
                string LockTime1 = unit_info.Lock_Info.LockYear.ToString("0000") + "-" + unit_info.Lock_Info.LockMonth.ToString("00") + "-" + unit_info.Lock_Info.LockDay.ToString("00") + " " + unit_info.Lock_Info.LockHour.ToString("00") + ":" + unit_info.Lock_Info.LockMin.ToString("00") + ":00";
                TimeSpan LockDay = Convert.ToDateTime(LockTime1).AddDays(LockTimeAll).Subtract(DateTime.Now);
                if (Convert.ToInt32(LockDay.Days) <= LockTimeAll) skinLabel23.Text = "试用剩余：" + LockDay.Days.ToString() + "天" + LockDay.Hours.ToString() + "时";
                else skinLabel23.Text = "试用期已到，请输入密钥解密！";
            }
            else /*if (unit_info.Lock_Info.LockNum >= 1)*/ skinLabel23.Text = "永久激活！";
#endif
            //================ 5 : Temp0-4 Value:温度0-4数值显示=================

            if (labTemp1Status.Text == "开启") labelTemp1.Text = unit_info.TempValue[0].ToString("00") + "℃";
            if (labTemp2Status.Text == "开启") labelTemp2.Text = unit_info.TempValue[1].ToString("00") + "℃";
            if (labTemp3Status.Text == "开启") labelTemp3.Text = unit_info.TempValue[2].ToString("00") + "℃";
            if (labTemp4Status.Text == "开启") labelTemp4.Text = unit_info.TempValue[3].ToString("00") + "℃";
            //================ 6 : Current0-6 Value:电流0-6显示==================
            tbxCurrent1.Text = unit_info.CurrentValue[0].ToString("00.0");
            tbxCurrent2.Text = unit_info.CurrentValue[1].ToString("00.0");
            tbxCurrent3.Text = unit_info.CurrentValue[2].ToString("00.0");
            tbxCurrent4.Text = unit_info.CurrentValue[3].ToString("00.0");
            tbxCurrent5.Text = unit_info.CurrentValue[4].ToString("00.0");
            tbxCurrent6.Text = unit_info.CurrentValue[5].ToString("00.0");
            //================ 7 : Temp-ProcessBar:温度计显示====================
            if (labTemp1Status.Text == "开启") progressBarTemp1.Value = unit_info.TempValue[0];
            if (labTemp2Status.Text == "开启") progressBarTemp2.Value = unit_info.TempValue[1];
            if (labTemp3Status.Text == "开启") progressBarTemp3.Value = unit_info.TempValue[2];
            if (labTemp4Status.Text == "开启") progressBarTemp4.Value = unit_info.TempValue[3];
            //================ 8 : chart display:折线图显示======================
            //更新X轴坐标时间
            DateTime t = DateTime.Now;
            chart1.ChartAreas[0].AxisX.CustomLabels.Clear();
            for (int i = 0; i <= 20; i++)
            {
                if (i % 4 == 0)
                {
                    CustomLabel label1 = new CustomLabel();
                    label1.ForeColor = Color.Black;

                    label1.Text = t.ToLongTimeString();
                    label1.ToPosition = i;
                    chart1.ChartAreas[0].AxisX.CustomLabels.Add(label1);
                    t = t.AddSeconds(1);
                }
            }
            //清除图像
            for (int i = 0; i < 10; i++) chart1.Series[i].Points.Clear();
            //更新数据数组
            for (int i = 0; i <= 99; i++) temp1[i] = temp1[i + 1]; temp1[100] = unit_info.TempValue[0];
            for (int i = 0; i <= 99; i++) temp2[i] = temp2[i + 1]; temp2[100] = unit_info.TempValue[1];
            for (int i = 0; i <= 99; i++) temp3[i] = temp3[i + 1]; temp3[100] = unit_info.TempValue[2];
            for (int i = 0; i <= 99; i++) temp4[i] = temp4[i + 1]; temp4[100] = unit_info.TempValue[3];
            for (int i = 0; i <= 99; i++) ix1[i] = ix1[i + 1]; ix1[100] = unit_info.CurrentValue[0];
            for (int i = 0; i <= 99; i++) ix2[i] = ix2[i + 1]; ix2[100] = unit_info.CurrentValue[1];
            for (int i = 0; i <= 99; i++) ix3[i] = ix3[i + 1]; ix3[100] = unit_info.CurrentValue[2];
            for (int i = 0; i <= 99; i++) ix4[i] = ix4[i + 1]; ix4[100] = unit_info.CurrentValue[3];
            for (int i = 0; i <= 99; i++) ix5[i] = ix5[i + 1]; ix5[100] = unit_info.CurrentValue[4];
            for (int i = 0; i <= 99; i++) ix6[i] = ix6[i + 1]; ix6[100] = unit_info.CurrentValue[5];
            //更新数据显示
            for (int i = 0; i <= 100; i++) chart1.Series[0].Points.AddXY((double)(i / 10.0), temp1[i] * 26 / 70.0);
            for (int i = 0; i <= 100; i++) chart1.Series[1].Points.AddXY((double)(i / 10.0), temp2[i] * 26 / 70.0);
            for (int i = 0; i <= 100; i++) chart1.Series[2].Points.AddXY((double)(i / 10.0), temp3[i] * 26 / 70.0);
            for (int i = 0; i <= 100; i++) chart1.Series[3].Points.AddXY((double)(i / 10.0), temp4[i] * 26 / 70.0);
            if(unit_info.DriverModeNum>=1) for (int i = 0; i <= 100; i++) chart1.Series[4].Points.AddXY((double)(i / 10.0), ix1[i]);
            if (unit_info.DriverModeNum >= 2) for (int i = 0; i <= 100; i++) chart1.Series[5].Points.AddXY((double)(i / 10.0), ix2[i]);
            if (unit_info.DriverModeNum >= 3) for (int i = 0; i <= 100; i++) chart1.Series[6].Points.AddXY((double)(i / 10.0), ix3[i]);
            if (unit_info.DriverModeNum >= 4) for (int i = 0; i <= 100; i++) chart1.Series[7].Points.AddXY((double)(i / 10.0), ix4[i]);
            if (unit_info.DriverModeNum >= 5) for (int i = 0; i <= 100; i++) chart1.Series[8].Points.AddXY((double)(i / 10.0), ix5[i]);
            if (unit_info.DriverModeNum >= 6) for (int i = 0; i <= 100; i++) chart1.Series[9].Points.AddXY((double)(i / 10.0), ix6[i]);
            if (unit_info.DriverModeNum >= 1) skinLabel37.Enabled = checkBox5.Enabled= true; else skinLabel37.Enabled = chart1.Series[4].Enabled = checkBox5.Enabled= checkBox5.Checked=false;
            if (unit_info.DriverModeNum >= 2) skinLabel38.Enabled = checkBox6.Enabled= true; else skinLabel38.Enabled = chart1.Series[5].Enabled= checkBox6.Enabled= checkBox6.Checked = false;
            if (unit_info.DriverModeNum >= 3) skinLabel48.Enabled =  checkBox7.Enabled=true; else skinLabel48.Enabled = chart1.Series[6].Enabled= checkBox7.Enabled= checkBox7.Checked = false;
            if (unit_info.DriverModeNum >= 4) skinLabel47.Enabled =checkBox8.Enabled=true; else skinLabel47.Enabled =  chart1.Series[7].Enabled = checkBox8.Enabled = checkBox8.Checked = false;
            if (unit_info.DriverModeNum >= 5) skinLabel52.Enabled =checkBox9.Enabled=true; else skinLabel52.Enabled = chart1.Series[8].Enabled= checkBox9.Enabled= checkBox9.Checked = false;
            if (unit_info.DriverModeNum >= 6) skinLabel51.Enabled = checkBox10.Enabled=true; else skinLabel51.Enabled = chart1.Series[9].Enabled= checkBox10.Enabled= checkBox10.Checked = false;
            //================ 9 : Alarm Status Display:报警功能显示=============

           string alarmText = null;
            if (unit_info.alarmFlag == true)
            {
                
                switch (unit_info.alarmCode)
                {
                    case 0x01: alarmText = "PD1"; break;
                    case 0x02: alarmText = "PD2"; break;
                    case 0x03: alarmText = "QBH"; break;
                    case 0x04: alarmText = "温度1"; break;
                    case 0x05: alarmText = "温度2"; break;
                    case 0x06: alarmText = "温度3"; break;
                    case 0x07: alarmText = "温度4"; break;
                    case 0x08: alarmText = "电流1"; break;
                    case 0x09: alarmText = "电流2"; break;
                    case 0x0A: alarmText = "电流3"; break;
                    case 0x0B: alarmText = "电流4"; break;
                    case 0x0C: alarmText = "电流5"; break;
                    case 0x0D: alarmText = "电流6"; break;
                    case 0x0E: alarmText = "水压"; break;
                    case 0x0F:alarmText = "急停";break;
                    case 0x10: alarmText = "更改参数"; break;
                    default: alarmText = "未定义"; break;
                }
                tbxAlarmInfo.AppendText(DateTime.Now.ToString() + ": " + alarmText + "报警！\r\n");
                tbxAlarmInfo.Focus();
                tbxAlarmInfo.Select(tbxAlarmInfo.TextLength, 0);
                tbxAlarmInfo.ScrollToCaret();
                unit_info.alarmFlag = false;
                //记录到文件中……

                while (AlarmFile_isLoad) ;
                AlarmFile_isOpen = true;
                StreamWriter alarm_sw = File.AppendText(AlarmFile_path);
                string alarm_info = DateTime.Now.Year.ToString("0000") + "-" + DateTime.Now.Month.ToString("00") + "-" +
                    DateTime.Now.Day.ToString("00") + " " + DateTime.Now.Hour.ToString("00") + ":" +
                    DateTime.Now.Minute.ToString("00") + ":" + DateTime.Now.Second.ToString("00") + " " + alarmText + "报警!" + " " + unit_info.TempValue[0].ToString("00") + ";" + unit_info.TempValue[1].ToString("00") +
                ";" + unit_info.TempValue[2].ToString("00") + ";" + unit_info.TempValue[3].ToString("00") + ";" + unit_info.CurrentValue[0].ToString("00.0") + ";" + unit_info.CurrentValue[1].ToString("00.0") + ";" +
                unit_info.CurrentValue[2].ToString("00.0") + ";" + unit_info.CurrentValue[3].ToString("00.0");
                alarm_sw.WriteLine(alarm_info);
                alarm_sw.Flush();
                alarm_sw.Close();
                AlarmFile_isOpen = false;
            }
            //================ 10 :Unit Para:设备参数显示========================
            tbxRedCurrent.Text = ((byte)(unit_info.Para_Read.Red_Current_Value * 4 / 4.7)).ToString();
            tbxPD1Value.Text = unit_info.Para_Read.PD1_Value.ToString();
            tbxPD2value.Text = unit_info.Para_Read.PD2_Value.ToString();
            tbxMaxCurrentvalue.Text = unit_info.Para_Read.Max_Current_Value.ToString();
            tbxCurrentValue.Text = unit_info.Para_Read.Warn_Current_Value.ToString();
            tbxTempValue.Text = unit_info.Para_Read.Warn_Temp_Value.ToString();
            alarmScreenLabel[0] = labPD1Status;
            alarmScreenLabel[1] = labPD2Status;
            alarmScreenLabel[2] = labQbhStatus;
            alarmScreenLabel[3] = labTemp1Status;
            alarmScreenLabel[4] = labTemp2Status;
            alarmScreenLabel[5] = labTemp3Status;
            alarmScreenLabel[6] = labTemp4Status;
            alarmScreenLabel[7] = labCurrent1Status;
            alarmScreenLabel[8] = labCurrent2Status;
            alarmScreenLabel[9] = labCurrent3Status;
            alarmScreenLabel[10] = labCurrent4Status;
            alarmScreenLabel[11] = labCurrent5Status;
            alarmScreenLabel[12] = labCurrent6Status;
            alarmScreenLabel[13] = labWatterStatus;
            for (byte i = 0; i < 14; i++)
            {
                if ((unit_info.Para_Read.Screen_Code & (1 << i)) == (1 << i))
                {
                    alarmScreenLabel[i].ForeColor = Color.Blue;
                    alarmScreenLabel[i].Text = "开启";
                }
                else
                {
                    alarmScreenLabel[i].ForeColor = Color.Red;
                    alarmScreenLabel[i].Text = "关闭";
                }
            }
            labQbhStatus1.ForeColor = labQbhStatus.ForeColor;
            labQbhStatus1.Text = labQbhStatus.Text;
            //温度1-4屏蔽显示
            if (labTemp1Status.Text == "关闭")
            {
                progressBarTemp1.Value = 0;
                labelTemp1.Text = "00℃";
                skinLabel25.Enabled = progressBarTemp1.Enabled = labelTemp1.Enabled = false;
                checkBox1.Checked = false;
                checkBox1.Enabled = false;
            }
            else
            {
                checkBox1.Enabled = true;
                skinLabel25.Enabled = progressBarTemp1.Enabled = labelTemp1.Enabled = true;
            }
            if (labTemp4Status.Text == "关闭")
            {
                progressBarTemp4.Value = 0;
                labelTemp4.Text = "00℃";
                skinLabel32.Enabled = progressBarTemp4.Enabled = labelTemp4.Enabled = false;
                checkBox4.Checked = false;
                checkBox4.Enabled = false;
            }
            else
            {
                checkBox4.Enabled = true;
                skinLabel32.Enabled = progressBarTemp4.Enabled = labelTemp4.Enabled = true;
            }
            if (labTemp2Status.Text == "关闭")
            {
                progressBarTemp2.Value = 0;
                labelTemp2.Text = "00℃";
                skinLabel28.Enabled = progressBarTemp2.Enabled = labelTemp2.Enabled = false;
                checkBox2.Checked = false;
                checkBox2.Enabled = false;
            }
            else
            {
                checkBox2.Enabled = true;
                skinLabel28.Enabled = progressBarTemp2.Enabled = labelTemp2.Enabled = true;
            }
            if (labTemp3Status.Text == "关闭")
            {
                progressBarTemp3.Value = 0;
                labelTemp3.Text = "00℃";
                skinLabel30.Enabled = progressBarTemp3.Enabled = labelTemp3.Enabled = false;
                checkBox3.Checked = false;
                checkBox3.Enabled = false;
            }
            else
            {
                checkBox3.Enabled = true;
                skinLabel30.Enabled = progressBarTemp3.Enabled = labelTemp3.Enabled = true;
            }

            //================ 11 :Unit Time:设备时间显示========================
            toolStripStatusLabel3.Text = unit_info.unit_time;

            //skinTextBox2.Text = unit_info.LogNum.ToString();
            //=====================  MCU历史报警 ===============================
            if (is_alarm_History) //报警请求MCU (内部20次)
            {
                is_alarm_History = false;
                if (err_code != 0)
                {
                    History_err.AppendText("20" + year.ToString() + "年" + month.ToString() + "月" + day.ToString() + "日" + hour.ToString() + "时" + ": " + get_type(err_code) + "报警! 0X" + err_code.ToString("X2") + "\r\n");
                }

                History_err.Focus();
                History_err.Select(History_err.TextLength, 0);
                History_err.ScrollToCaret();
            }
            //================== SN号解锁 ==============
            if (unit_info.UnitSN == "A000000000")
            {
                ParaSetUnlock();
                skinButton30.ForeColor = Color.Green;
                skinButton30.Text = "已登录！!";
            }


        }

        /// <summary>
        /// 定时器500ms，中断处理函数
        /// </summary>
        public delegate void interfaceRefresh();    //创建委托
        public interfaceRefresh DisRefresh; //定义一个委托，在窗体导入函数里面声明：用于界面显示更新
        private void timer1_Tick(object sender, EventArgs e)
        {
            this.BeginInvoke(DisRefresh);
        }

        /// <summary>
        /// chart表 线路选择显示
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void checkBox11_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox11.Checked == true)
            {
                
                if (labTemp1Status.Text == "开启") checkBox1.Checked = true;
                if (labTemp2Status.Text == "开启") checkBox2.Checked = true;
                if (labTemp3Status.Text == "开启") checkBox3.Checked = true;
                if (labTemp4Status.Text == "开启") checkBox4.Checked = true;
                if (unit_info.DriverModeNum >= 1) checkBox5.Checked = true;
                if (unit_info.DriverModeNum >= 2) checkBox6.Checked = true;
                if (unit_info.DriverModeNum >= 3) checkBox7.Checked = true;
                if (unit_info.DriverModeNum >= 4) checkBox8.Checked = true;
                if (unit_info.DriverModeNum >= 5) checkBox9.Checked = true;
                if (unit_info.DriverModeNum >= 6) checkBox10.Checked = true;
            }
            else
            {
                checkBox1.Checked = checkBox2.Checked = checkBox3.Checked = checkBox4.Checked = checkBox5.Checked = checkBox6.Checked =
                    checkBox7.Checked = checkBox8.Checked = checkBox9.Checked = checkBox10.Checked = false;
            }
        }
        //T1
        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox1.Checked) chart1.Series[0].Enabled = true; else chart1.Series[0].Enabled = false;
        }
        //T2
        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked) chart1.Series[1].Enabled = true; else chart1.Series[1].Enabled = false;
        }
        //T3
        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox3.Checked) chart1.Series[2].Enabled = true; else chart1.Series[2].Enabled = false;
        }
        //T4
        private void checkBox4_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox4.Checked) chart1.Series[3].Enabled = true; else chart1.Series[3].Enabled = false;
        }
        //I1
        private void checkBox5_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox5.Checked) chart1.Series[4].Enabled = true; else chart1.Series[4].Enabled = false;
        }
        //I2
        private void checkBox6_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox6.Checked) chart1.Series[5].Enabled = true; else chart1.Series[5].Enabled = false;
        }
        //I3
        private void checkBox7_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox7.Checked) chart1.Series[6].Enabled = true; else chart1.Series[6].Enabled = false;
        }
        //I4
        private void checkBox8_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox8.Checked) chart1.Series[7].Enabled = true; else chart1.Series[7].Enabled = false;
        }
        //I5
        private void checkBox9_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox9.Checked) chart1.Series[8].Enabled = true; else chart1.Series[8].Enabled = false;
        }
        //I6
        private void checkBox10_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox10.Checked) chart1.Series[9].Enabled = true; else chart1.Series[9].Enabled = false;
        }


        public void ParaSetLock()
        {
            btnPD1On.Enabled = btnPD1Close.Enabled =
                btnPD2On.Enabled = btnPD2Close.Enabled =
                btnQbhOpen1.Enabled = btnQbhClose1.Enabled =
                btnWatterOn.Enabled = btnWatterOff.Enabled =
                btnTemp1On.Enabled = btnTemp1Close.Enabled =
                btnTemp2On.Enabled = btnTemp2Close.Enabled =
                btnTemp3On.Enabled = btnTemp3Close.Enabled =
                btnTemp4On.Enabled = btnTemp4Off.Enabled =
                btnCurrent1On.Enabled = btnCurrent1Off.Enabled =
                btnCurrent2On.Enabled = btnCurrent2Off.Enabled =
                btnCurrent3On.Enabled = btnCurrent3Off.Enabled =
                btnCurrent4On.Enabled = btnCurrent4Off.Enabled =
                btnCurrent5On.Enabled = btnCurrent5Off.Enabled =
                btnCurrent6On.Enabled = btnCurrent6Off.Enabled =
                btnAllAlarmOn.Enabled = btnAllAlarmOff.Enabled =
                btnRedCurrentSet.Enabled =
                btnPD1Set.Enabled =
                btnPD2Set.Enabled =
                btnMaxCurrentSet.Enabled =
                btnTempSet.Enabled =
                btnCurrentSet.Enabled =
                btnSetSN.Enabled =
                btnParaLockSet.Enabled =
                btnGetLocalTime.Enabled =
                btnUnityTimeSet.Enabled = false;

        }

        public void ParaSetUnlock()
        {
            btnPD1On.Enabled = btnPD1Close.Enabled =
                btnPD2On.Enabled = btnPD2Close.Enabled =
                btnQbhOpen1.Enabled = btnQbhClose1.Enabled =
                btnWatterOn.Enabled = btnWatterOff.Enabled =
                btnTemp1On.Enabled = btnTemp1Close.Enabled =
                btnTemp2On.Enabled = btnTemp2Close.Enabled =
                btnTemp3On.Enabled = btnTemp3Close.Enabled =
                btnTemp4On.Enabled = btnTemp4Off.Enabled =
                btnCurrent1On.Enabled = btnCurrent1Off.Enabled =
                btnCurrent2On.Enabled = btnCurrent2Off.Enabled =
                btnCurrent3On.Enabled = btnCurrent3Off.Enabled =
                btnCurrent4On.Enabled = btnCurrent4Off.Enabled =
                btnCurrent5On.Enabled = btnCurrent5Off.Enabled =
                btnCurrent6On.Enabled = btnCurrent6Off.Enabled =
                btnAllAlarmOn.Enabled = btnAllAlarmOff.Enabled =
                btnRedCurrentSet.Enabled =
                btnPD1Set.Enabled =
                btnPD2Set.Enabled =
                btnMaxCurrentSet.Enabled =
                btnTempSet.Enabled =
                btnCurrentSet.Enabled =
                btnSetSN.Enabled =
                btnParaLockSet.Enabled =
                btnGetLocalTime.Enabled =
                btnUnityTimeSet.Enabled = true;
        }

        //登录参数设置界面
        public void skinButton30_Click(object sender, EventArgs e)
        {
            if (skinTextBox2.Text == RSA(143, 11, key_log, 1))
            {
                key_log = unit_info.LogNum.ToString("000") + labelSN.Text;
                ParaSetUnlock();
                skinButton30.ForeColor = Color.Green;
                skinButton30.Text = "已登录！";
            }
            else MessageBox.Show("密码错误！");
    
        }

        //加密

#region INI文件相关驱动函数
        // 声明读取ini文件的函数
        [DllImport("kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string val, string filepath);
        [DllImport("kernel32")]
        private static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retval, int size, string filePath);

        // 读Ini文件
        public static string ReadIniData(string Section, string Key, string NoText, string iniFilePath)
        {
            StringBuilder temp = new StringBuilder(1024);
            GetPrivateProfileString(Section, Key, NoText, temp, 1024, iniFilePath);
            return temp.ToString();
        }

        // 写Ini文件
        public static bool WriteIniData(string Section, string Key, string Value, string iniFilePath)
        {
            long OpStation = WritePrivateProfileString(Section, Key, Value, iniFilePath);
            if (OpStation == 0) return false; else return true;
        }
#endregion

        public char GetChar(int num)
        {
            char[] asiCode = { '9', '8', '7', '6', '5', '4', '3', '2', '1', '0',
                               'F', 'L', 'Q', 'V', 'A', 'B', 'C', 'D', 'E', 'F',
                               'E', 'K', 'P', 'U', 'Z', 'K', 'J', 'I', 'H', 'G',
                               'D', 'J', 'O', 'T', 'Y', 'L', 'M', 'N', 'O', 'P',
                               'C', 'I', 'N', 'S', 'X', 'U', 'T', 'S', 'R', 'Q',
                               'B', 'H', 'M', 'R', 'W', 'Z', 'Y', 'X', 'W', 'V',
                               'A', 'G' };
            return asiCode[num];
        }
        //RSA加密算法
        public string RSA(int baseNum, int key, string message, int num)
        {
            string rsaMessage = null;
            foreach (char s in message)
            {
                rsaMessage += GetChar((Convert.ToInt16(Math.Round(Math.Pow((int)s + num, key)) % baseNum)) % 62);
            }
            return rsaMessage;
        }

        private void btnKeyApp_Click(object sender, EventArgs e)
        {
            string SerialNumber = labelSN.Text;
            int[] key = { 7, 11, 19, 13, 37, 103, 29, 31, 0, 0 };
            byte time_save = Convert.ToByte(ReadIniData("CIPHER CODE", "time_used", "255", ini_path));
            byte time = 0;
            byte key_type = 0;
            for (byte i = 0; i < 8; i++)
            {
                for (byte j = 1; j <= 50; j++)
                {
                    if (tbxKey.Text.CompareTo(RSA(143, key[i], SerialNumber, j)) == 0)
                    {
                        time = j;
                        key_type = (byte)(i);
                        break;
                    }
                }
            }

            if (key_type == 7)
            {
                byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x09, 0x00, 0x00 };
                cmd[9] = key_type;
                com.RS232_Send(cmd);
            }
            else
            {
                if ((unit_info.Lock_Info.LockNum + 1) == time)
                {
                    byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x09, 0x00, 0x00 };
                    cmd[9] = key_type;
                    com.RS232_Send(cmd);
                    WriteIniData("CIPHER CODE", "time_used", (time_save + 1).ToString(), ini_path);
                    //label26.Text = "已解密" + ReadIniData("CIPHER CODE", "time_used", "255", ini_path) + "次";

                }
                else
                {
                    MessageBox.Show("激活码错误或者过期！");
                }
            }
        }

        public string key_log = "0";

        private void skinButton37_Click(object sender, EventArgs e)
        {
            key_log = unit_info.LogNum.ToString("000") + labelSN.Text;
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x08, 0x00, 0x00 };
            com.RS232_Send(cmd);
            MessageBox.Show("动态码："+ key_log);

        }
        
        //打开历史记录文件
        private void button1_Click(object sender, EventArgs e)
        {
            AlarmFile_isLoad = true;
            while (AlarmFile_isOpen == true) ;
            OpenFileDialog ofd = new OpenFileDialog();
            //ofd.InitialDirectory = System.Environment.CurrentDirectory;
            ofd.InitialDirectory = AlarmFile_base_path;
            ofd.Filter = "Text Document(*.txt)|*.txt|All Files|*.*";

            ofd.ShowDialog();
            string path1 = ofd.FileName;

            if (path1 != "")
            {
                StreamReader sr = new StreamReader(path1);
                string read_str;
                dataGridView1.Rows.Clear();//清除datagirdview显示
                while ((read_str = sr.ReadLine()) != null)
                {
                    string[] text = read_str.Split(' ');
                    int index = dataGridView1.Rows.Add();
                    dataGridView1.Rows[index].Cells[0].Value = text[0];
                    dataGridView1.Rows[index].Cells[1].Value = text[1];
                    dataGridView1.Rows[index].Cells[2].Value = text[2];
                    dataGridView1.Rows[index].Cells[3].Value = text[3];
                }
                dataGridView1.FirstDisplayedScrollingRowIndex = dataGridView1.RowCount - 1;
                sr.Close();
            }

            AlarmFile_isLoad = false;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            History_err.Text = null;
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x06, 0x00 };
            com.RS232_Send(cmd);
        }

        public static string get_type(byte dat)
        {
            string errText = null;
            switch (dat)
            {
                case 0x01: errText = "PD1"; break;
                case 0x02: errText = "PD2"; break;
                case 0x03: errText = "QBH"; break;
                case 0x04: errText = "温度1"; break;
                case 0x05: errText = "温度2"; break;
                case 0x06: errText = "温度3"; break;
                case 0x07: errText = "温度4"; break;
                case 0x08: errText = "电流1"; break;
                case 0x09: errText = "电流2"; break;
                case 0x0A: errText = "电流3"; break;
                case 0x0B: errText = "电流4"; break;
                case 0x0C: errText = "电流5"; break;
                case 0x0D: errText = "电流6"; break;
                case 0x0E: errText = "水压"; break;
                case 0x0F: errText = "急停"; break;
                case 0x10: errText = "参数改变"; break;
                case 0x11: errText = "PD报警锁机"; break;
                case 0xFF: errText = "解密记录"; break;
                default: errText = "未知错误"; break;
            }
            return errText;
        }

        public bool is_alarm_History = false;

        private void skinTabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (skinTabControl1.SelectedIndex != 1)
            {
                ParaSetLock();
                skinButton30.ForeColor = Color.Red;
                skinButton30.Text = "设置上锁，请先登录!";
            }
            if (unit_info.UnitSN == "A000000000")
            {
                ParaSetUnlock();
                skinButton30.ForeColor = Color.Green;
                skinButton30.Text = "已登录！!";
            }
        }

        //开启红光
        private void skinButton38_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x06, 0x01, 0x01, 0x00 };
            com.RS232_Send(cmd);
        }
        //关闭红光
        private void skinButton39_Click(object sender, EventArgs e)
        {
            byte[] cmd = { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x06, 0x01, 0x00, 0x00 };
            com.RS232_Send(cmd);
        }
    }

    #region 双缓冲Panel
    public class DoubleBufferPanel : Panel
    {
        public DoubleBufferPanel()
        {
            this.SetStyle(ControlStyles.AllPaintingInWmPaint | //不擦除背景 ,减少闪烁
                 ControlStyles.OptimizedDoubleBuffer | //双缓冲
                 ControlStyles.UserPaint, //使用自定义的重绘事件,减少闪烁
                 true);
        }
    }
#endregion

#region 双缓冲dataGirdView
    public static class DoubleBufferDataGridView
    {
        public static void DoubleBufferedDataGirdView(this DataGridView dgv, bool flag)
        {
            Type dgvType = dgv.GetType();
            PropertyInfo pi = dgvType.GetProperty("DoubleBuffered", BindingFlags.Instance | BindingFlags.NonPublic);
            pi.SetValue(dgv, flag, null);
        }
    }
#endregion

#region 双缓冲tabControl
    public static class DoubleBuffertabControl
    {
        public static void DoubleBufferedtabControl(this TabControl dgv, bool flag)
        {
            Type dgvType = dgv.GetType();
            PropertyInfo pi = dgvType.GetProperty("DoubleBuffered", BindingFlags.Instance | BindingFlags.NonPublic);
            pi.SetValue(dgv, flag, null);
        }
    }
#endregion
}
