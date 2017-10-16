#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QXmlStreamReader>
#include<QXmlStreamWriter>
#include<QFile>
#include<QDir>
#include<QTextCodec>
#include<QTime>
#include<QList>
#include<QInputDialog>
#include<QListWidgetItem>
#include<QDebug>
#include<QMessageBox>

namespace Ui {
class MainWindow;
}

class Novel
{
public:
    QString novelName;
};
class Text
{
public:
    QString novelName;
    QString zhangName;
    QString fileName;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //初始化，打开软件首先调用,初始化“编码格式”、“测试路径”、“标志变量：MainWindowPageFlag”
    void Init();
    //初始化编码格式为utf-8
    void InitEncode();

    //获取xml文件目录，若不在则创建 例如：QDir::tempPath+"/LinuxXieZuo/Info.xml"
    QString getXmlDir();

    //获取TEXT文件的目录，若不存在则创建 例如：QDir::tempPath+"/LinuxXieZuo/text"
    QString getTextDir();

    //获取一个由时间+".txt"组成的字符串 例如: 20171011121311.txt
    QString getTextNameByTime();
    QString getTextNameByXml(QString novelName,QString zhangName);//在xml中获取文件名

    //获取xml字符目录
    QStringList getNovelList(QString XmlPath);
    QStringList getZhangList(QString novelName,QString XmlPath);

    //新建项 0:成功 -1:重名，创建失败
    int newNovelItem(QString novelName);
    int newZhangItem(QString zhangName,QString novelName);

    //删除项 0:成功 -1:找不到该项 -2含有子项未删除
    int deleteNovelItem(QString novelName);
    int deleteZhangItem(QString zhangName,QString novelName);

    ///////
    /// 以下为响应事件的函数
    ///////

    void selectItem(QString selectStr);//ListWidget项被选中时，改变一些可修改变量
    void newItemButtonClick();//新建按钮被选中时进行响应，注意判断MAINWINDOWPAGEFLAG标志变量
    void deleteItemButtonClick();//删除按钮被选中时进行响应
    void backItemButtonClick();//后退按钮被选中时响应
    void selectItemButtonClick(QString selectedItem);//选择按钮被点击时响应,该按钮只存在于list页面上
    void saveTextButtonClick();//保存按钮点击响应，该按钮只存在于文本编辑页面上
    void ClearAll();//清空所有痕迹，初始化该软件

    //更改背景色
    QStringList getStyleNameList();//获取css样式名称列表 style_name 的list
    QString getStyleFileName(QString styleName);//获取css样式的文件名 参数，样式名称
    int setStyleByCSS(QString styleCSSFileName);

    //一些常量，各种地址
#define MAIN_DIR "/LinuxXieZuo"
#define XML_DIR "/LinuxXieZuo/Info.xml"
#define TEXT_DIR "/LinuxXieZuo/text"
#define NOVEL_SELECT 0
#define ZHANG_SELECT 1
#define TEXT_EDIT 2

#define TEXT_HAVESAVED 0
#define TEXT_NOSAVED 1
    //一些可修改的变量
    QString selectedNovelName;//选中的小说名称
    QString selectedZhangName;//选中的章节名称
    QString selectedTextFileName;//选中章节的文本地址,一般为："20171011121311.txt"

    int MainWindowPageFlag;
    int isTextChanged;//0:当前文本编辑页面已经保存 1：当前文本编辑页面未保存
private slots:
    void on_newButton_clicked();

    void on_deleteButton_clicked();

    void on_selecteButton_clicked();


    void on_saveButton_clicked();

    void on_backButton_inTextPage_clicked();

    void on_backButton_clicked();

    void on_textEdit_textChanged();

    void on_select_listWidget_itemClicked(QListWidgetItem *item);

    void on_textSize_spinBox_valueChanged(int arg1);

    void on_initAll_action_triggered(bool checked);

    void on_daBao_action_triggered(bool checked);

    void on_changeStyle_action_triggered(bool checked);

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
