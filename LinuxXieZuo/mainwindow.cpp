#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Init();
    QObject::connect(ui->select_listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),ui->selecteButton,SLOT(click()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Init()
{
    InitEncode();
    QString xmlpath=getXmlDir();
    QString textdir=getTextDir();
    MainWindowPageFlag = NOVEL_SELECT;//设置初始页面为小说选择页面
    ui->stackedWidget->setCurrentIndex(0);//设置main_page为初始页
    ui->select_listWidget->clear();
    ui->select_listWidget->addItems(getNovelList(xmlpath));
    ui->showListType_label->setText("小说:");//设置初始为“小说”
    setStyleByCSS(getStyleFileName("普通"));//设置样式
}

void MainWindow::InitEncode()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//情况2
    QTextCodec::setCodecForTr(codec);//设置用于QObject的编解码器：：tr()对其参数为codec.
    QTextCodec::setCodecForLocale(codec);//设置编解码器为codec
    QTextCodec::setCodecForCStrings(codec);//集QString转换和从const char *和qbytearrays使用编解码器 codec
}

QString MainWindow::getXmlDir()
{
    QString mainDir = QDir::homePath()+MAIN_DIR;
    QString xml_dir = QDir::homePath()+XML_DIR;
    QFile f(xml_dir);
    QDir dir(mainDir);
    if(!dir.exists())
    {
        dir.mkpath(mainDir);
    }
    if(!f.exists())
    {
        f.open(QIODevice::WriteOnly);
        f.close();

        QXmlStreamWriter writer;
        f.open(QIODevice::WriteOnly);
        writer.setDevice(&f);
        writer.writeStartDocument();
        writer.writeStartElement("texts");
        writer.writeEndElement();
        writer.writeEndDocument();
        f.close();
    }
    return xml_dir;
}
QString MainWindow::getTextDir()
{
    QString textPath=QDir::homePath()+TEXT_DIR;
    QDir dir(textPath);
    if(!dir.exists())
        dir.mkpath(textPath);
    return textPath;
}
QString MainWindow::getTextNameByTime()
{
    QDateTime time=QDateTime::currentDateTime();
    QString timeStr = time.toString("yyyyMMddhhmmss");
    return timeStr+".txt";
}

QStringList MainWindow::getNovelList(QString XmlPath)
{
    QFile f(XmlPath);

    f.open(QIODevice::ReadOnly);
    QXmlStreamReader reader;
    reader.setDevice(&f);
    QStringList novelNameList;
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name()=="novel")
            {
                QString str = reader.attributes().value("novel_name").toString();
                if(!novelNameList.contains(str))
                {
                    novelNameList.append(str);
                }
            }
        }
        reader.readNext();
    }
    return novelNameList;
}
QStringList MainWindow::getZhangList(QString novelName, QString XmlPath)
{
    QFile f(XmlPath);
    f.open(QIODevice::ReadOnly);
    QXmlStreamReader reader;
    reader.setDevice(&f);
    QStringList zhangNameList;
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name()=="text"&&reader.attributes().value("novel_name").toString() == novelName)
            {
                zhangNameList.append(reader.attributes().value("zhang_name").toString());
            }
        }
        reader.readNext();
    }
    return zhangNameList;
}

int MainWindow::newNovelItem(QString novelName)
{
    QString path = getXmlDir();
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QXmlStreamReader reader;
    reader.setDevice(&f);
    QList<Novel> novelList;
    QList<Text> textList;
    //循环读取小说和章节信息
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name().toString() == "novel")
            {
                Novel newNovel;
                newNovel.novelName = reader.attributes().value("novel_name").toString();
                if(novelName == newNovel.novelName)
                {
                    f.close();
                    return -1;//错误1 重名
                }
                novelList.append(newNovel);
            }
            else if(reader.name().toString() == "text")
            {
                Text newText;
                newText.novelName = reader.attributes().value("novel_name").toString();
                newText.zhangName = reader.attributes().value("zhang_name").toString();
                newText.fileName = reader.attributes().value("file_name").toString();
                textList.append(newText);
            }
        }
        reader.readNext();
    }
    //将新建的小说压入列表
    Novel createNovel;

    createNovel.novelName = novelName;

    novelList.append(createNovel);
    //换为写模式
    f.close();
    f.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer;
    writer.setDevice(&f);
    //先写小说信息
    writer.writeStartDocument();
    writer.writeStartElement("texts");
    for(int i=0;i<novelList.length();i++)
    {
        writer.writeStartElement("novel");
        writer.writeAttribute("novel_name",novelList.at(i).novelName);
        writer.writeEndElement();
    }
    //再写章节信息
    for(int i=0;i<textList.length();i++)
    {
        writer.writeStartElement("text");
        writer.writeAttribute("novel_name",textList.at(i).novelName);
        writer.writeAttribute("zhang_name",textList.at(i).zhangName);
        writer.writeAttribute("file_name",textList.at(i).fileName);
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();
    f.close();

    return 0;
}
int MainWindow::newZhangItem(QString zhangName, QString novelName)//新建章节，注意章节（text）重名问题
{
    QString path = getXmlDir();
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QXmlStreamReader reader;
    reader.setDevice(&f);
    QList<Novel> novelList;
    QList<Text> textList;
    //循环读取小说和章节信息
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name().toString() == "novel")
            {
                Novel newNovel;
                newNovel.novelName = reader.attributes().value("novel_name").toString();
                novelList.append(newNovel);
            }
            else if(reader.name().toString() == "text")
            {
                if(reader.attributes().value("novel_name").toString() == novelName
                        && reader.attributes().value("zhang_name").toString() == zhangName)
                {
                    f.close();

                    return -1;//错误1 章节重名
                }
                Text newText;
                newText.novelName = reader.attributes().value("novel_name").toString();
                newText.zhangName = reader.attributes().value("zhang_name").toString();
                newText.fileName = reader.attributes().value("file_name").toString();
                textList.append(newText);
            }
        }
        reader.readNext();
    }
    //将新建的章节压入列表
    Text createText;
    createText.novelName = novelName;
    createText.zhangName = zhangName;
    createText.fileName = getTextNameByTime();
    textList.append(createText);
    //在文件夹内创建名为create.fileName的文本，
    QFile createTextFile(getTextDir()+"/"+createText.fileName);
    createTextFile.open(QIODevice::WriteOnly);
    createTextFile.write("");
    createTextFile.close();
    //换为写模式
    f.close();
    f.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer;
    writer.setDevice(&f);
    //先写小说信息
    writer.writeStartDocument();
    writer.writeStartElement("texts");
    for(int i=0;i<novelList.length();i++)
    {
        writer.writeStartElement("novel");
        writer.writeAttribute("novel_name",novelList.at(i).novelName);
        writer.writeEndElement();
    }
    //再写章节信息
    for(int i=0;i<textList.length();i++)
    {
        writer.writeStartElement("text");
        writer.writeAttribute("novel_name",textList.at(i).novelName);
        writer.writeAttribute("zhang_name",textList.at(i).zhangName);
        writer.writeAttribute("file_name",textList.at(i).fileName);
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();
    f.close();
    return 0;
}

int MainWindow::deleteNovelItem(QString novelName)
{
    QStringList zhangStrList = getZhangList(novelName,getXmlDir());
    QStringList novelStrList = getNovelList(getXmlDir());
    if(zhangStrList.length()>0)//错误2 存在章节子项无法删除该小说
    {
        QMessageBox::warning(this,tr("无法删除！"),tr("请先删除该小说的所有章节。"),QMessageBox::Ok);
        return -2;
    }
    if(!novelStrList.contains(novelName))//错误1 未找到该小说
    {

        return -1;
    }
    QString path = getXmlDir();
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QXmlStreamReader reader;
    reader.setDevice(&f);
    QList<Novel> novelList;
    QList<Text> textList;
    //循环读取小说和章节信息
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name().toString() == "novel")
            {
                Novel newNovel;
                newNovel.novelName = reader.attributes().value("novel_name").toString();
                novelList.append(newNovel);
            }
            else if(reader.name().toString() == "text")
            {
                Text newText;
                newText.novelName = reader.attributes().value("novel_name").toString();
                newText.zhangName = reader.attributes().value("zhang_name").toString();
                newText.fileName = reader.attributes().value("file_name").toString();
                textList.append(newText);
            }
        }
        reader.readNext();
    }
    //将需要删除的小说项在列表中删除
    for(int i=0;i<novelList.length();i++)
    {
        if(novelList.at(i).novelName == novelName)
        {
            novelList.removeAt(i);
            break;
        }
    }
    //换为写模式
    f.close();
    f.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer;
    writer.setDevice(&f);
    //先写小说信息
    writer.writeStartDocument();
    writer.writeStartElement("texts");
    for(int i=0;i<novelList.length();i++)
    {
        writer.writeStartElement("novel");
        writer.writeAttribute("novel_name",novelList.at(i).novelName);
        writer.writeEndElement();
    }
    //再写章节信息
    for(int i=0;i<textList.length();i++)
    {
        writer.writeStartElement("text");
        writer.writeAttribute("novel_name",textList.at(i).novelName);
        writer.writeAttribute("zhang_name",textList.at(i).zhangName);
        writer.writeAttribute("file_name",textList.at(i).fileName);
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();
    f.close();

    return 0;
}

int MainWindow::deleteZhangItem(QString zhangName, QString novelName)
{

    QString xmlPath = getXmlDir();
    QStringList zhangStrList = getZhangList(novelName,xmlPath);
    QStringList novelStrList = getNovelList(xmlPath);
    if(!zhangStrList.contains(zhangName))//错误1 不存在此章节
    {
        return -1;
    }
    if(!novelStrList.contains(novelName))//错误2 未找到该小说
    {
        return -2;
    }

    QFile f(xmlPath);
    f.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;
    reader.setDevice(&f);
    QList<Novel> novelList;
    QList<Text> textList;
    //循环读取小说和章节信息
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name().toString() == "novel")
            {
                Novel newNovel;
                newNovel.novelName = reader.attributes().value("novel_name").toString();
                novelList.append(newNovel);
            }
            else if(reader.name().toString() == "text")
            {
                Text newText;
                newText.novelName = reader.attributes().value("novel_name").toString();
                newText.zhangName = reader.attributes().value("zhang_name").toString();
                newText.fileName = reader.attributes().value("file_name").toString();
                textList.append(newText);
            }
        }
        reader.readNext();
    }
    //遍历查找
    for (int i=0;i<textList.length();i++)
    {
        if(textList.at(i).novelName == novelName && textList.at(i).zhangName == zhangName)
        {
            //找到了需要删除的章节，1.删除章节所属的文本文件；2.在列表中删除章节的信息
            QFile f(getTextDir()+"/"+textList.at(i).fileName);

            f.remove();
            textList.removeAt(i);//2
            break;
        }
    }

    //换为写模式
    f.close();
    f.open(QIODevice::WriteOnly);
    QXmlStreamWriter writer;
    writer.setDevice(&f);
    //先写小说信息
    writer.writeStartDocument();
    writer.writeStartElement("texts");
    for(int i=0;i<novelList.length();i++)
    {
        writer.writeStartElement("novel");
        writer.writeAttribute("novel_name",novelList.at(i).novelName);
        writer.writeEndElement();
    }
    //再写章节信息
    for(int i=0;i<textList.length();i++)
    {
        writer.writeStartElement("text");
        writer.writeAttribute("novel_name",textList.at(i).novelName);
        writer.writeAttribute("zhang_name",textList.at(i).zhangName);
        writer.writeAttribute("file_name",textList.at(i).fileName);
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();
    f.close();

    return 0;
}

QString MainWindow::getTextNameByXml(QString novelName, QString zhangName)
{
    QXmlStreamReader reader;
    QFile f(getXmlDir());
    f.open(QIODevice::ReadOnly);
    reader.setDevice(&f);
    while(!reader.atEnd())
    {
        if(reader.isStartElement())
        {
            if(reader.name().toString() == "text" && reader.attributes().value("novel_name").toString() == novelName
                    && reader.attributes().value("zhang_name").toString() == zhangName)
            {
                QString fileName = reader.attributes().value("file_name").toString();
                f.close();
                return fileName;
            }
        }
        reader.readNext();
    }
    return "";
}

void MainWindow::selectItem(QString selectStr)
{

    if(MainWindowPageFlag == NOVEL_SELECT)
        selectedNovelName = selectStr;
    else if(MainWindowPageFlag == ZHANG_SELECT)
    {

        selectedZhangName = selectStr;
        selectedTextFileName = getTextNameByXml(selectedNovelName,selectedZhangName);
    }
}

void MainWindow::newItemButtonClick()
{
    //注意判断page标志
    QString title="新建",label;
    if(MainWindowPageFlag == NOVEL_SELECT)
        label = "请在输入框输入新的小说名称";
    else
        label = "请输入新的章节名称";
    bool ok;
    QString inputStr = QInputDialog::getText(this,title,label,QLineEdit::Normal,"",&ok);
    if(ok && !inputStr.isEmpty())
    {
        int flag;
        if(MainWindowPageFlag == NOVEL_SELECT)
        {
            flag = newNovelItem(inputStr);
            if(flag == 0)
            {
                ui->select_listWidget->clear();
                ui->select_listWidget->addItems(getNovelList(
                                                    getXmlDir()));
            }
        }
        else
        {
            flag = newZhangItem(inputStr,selectedNovelName);
            if(flag == 0)
            {
                ui->select_listWidget->clear();
                ui->select_listWidget->addItems(getZhangList(
                                                    selectedNovelName,
                                                    getXmlDir()
                                                    )
                                                );
            }
        }
        ui->select_listWidget->item(ui->select_listWidget->count()-1)->setSelected(true);
    }
}

void MainWindow::deleteItemButtonClick()
{
    int flag = 0;
    if(MainWindowPageFlag == NOVEL_SELECT)
    {
        flag = deleteNovelItem(selectedNovelName);
        if(flag == 0)
        {
            ui->select_listWidget->clear();
            ui->select_listWidget->addItems(
                        getNovelList(
                            getXmlDir()
                            )
                        );
        }
    }
    else
    {
        flag = deleteZhangItem(selectedZhangName,selectedNovelName);
        if(flag == 0)
        {
            ui->select_listWidget->clear();
            ui->select_listWidget->addItems(getZhangList(selectedNovelName,getXmlDir()));
        }
    }
}

void MainWindow::backItemButtonClick()
{
    if(MainWindowPageFlag == NOVEL_SELECT)
    {
        if(QMessageBox::Ok == QMessageBox::warning(this,"退出程序","您确认退出吗？",QMessageBox::Ok,QMessageBox::Cancel))
            this->close();
        return;
    }
    else if(MainWindowPageFlag == ZHANG_SELECT)
    {
        MainWindowPageFlag = NOVEL_SELECT;//改为小说选择界面
        ui->select_listWidget->clear();
        ui->select_listWidget->addItems(
                    getNovelList(
                        getXmlDir()
                        )
                    );

    }
    else //MainWindowPageFlag == TEXT_EDIT
    {
        if(isTextChanged == TEXT_NOSAVED)
        {
            //先保存一下文本
            QString text = ui->textEdit->toPlainText();
            QFile f(getTextDir()+"/"+selectedTextFileName);
            f.open(QIODevice::WriteOnly);
            f.write(text.toUtf8());//写入textedit里的文本到目录下的txt文件内，覆盖
            f.close();
        }
        MainWindowPageFlag = ZHANG_SELECT;
        ui->stackedWidget->setCurrentIndex(0);
        ui->select_listWidget->clear();
        ui->select_listWidget->addItems(getZhangList(selectedNovelName,getXmlDir()));
    }
}

void MainWindow::selectItemButtonClick(QString selectedItem)
{

    if(MainWindowPageFlag == NOVEL_SELECT)
    {
        //转换到章节选择上
        MainWindowPageFlag = ZHANG_SELECT;
        selectedNovelName = selectedItem;
        ui->select_listWidget->clear();
        ui->select_listWidget->addItems(getZhangList(selectedNovelName,
                                                     getXmlDir()));
    }
    else if(MainWindowPageFlag == ZHANG_SELECT)
    {
        MainWindowPageFlag = TEXT_EDIT;
        selectedZhangName = selectedItem;
        selectedTextFileName = getTextNameByXml(selectedNovelName,selectedZhangName);

        QFile f(getTextDir()+"/"+selectedTextFileName);
        f.open(QIODevice::ReadOnly);
        QString strInFile = f.readAll();
        ui->textEdit->clear();
        ui->textEdit->append(strInFile);
        ui->stackedWidget->setCurrentIndex(1);//设置text页显示

    }
}

void MainWindow::saveTextButtonClick()
{
    QString str_HaveSaved = ui->textEdit->toPlainText();
    QFile f(getTextDir()+"/"+selectedTextFileName);
    f.open(QIODevice::WriteOnly);
    f.write(str_HaveSaved.toUtf8());
    f.close();
}

void MainWindow::ClearAll()
{
    QDir textDir(getTextDir());
    //清空text目录下的所有文件
    textDir.setFilter(QDir::Files);
    for(int i=0;i<textDir.count();i++)
    {
        textDir.remove(textDir[i]);
    }
    //重写info.xml
    QFile f(getXmlDir());
    QXmlStreamWriter writer;
    f.open(QIODevice::WriteOnly);
    writer.setDevice(&f);
    writer.writeStartDocument();
    writer.writeStartElement("texts");
    writer.writeEndElement();
    writer.writeEndDocument();
    f.close();
    Init();//初始化为软件打开状态
}

void MainWindow::on_newButton_clicked()
{
    newItemButtonClick();
}

void MainWindow::on_deleteButton_clicked()
{
    deleteItemButtonClick();
}

void MainWindow::on_selecteButton_clicked()
{

    if(ui->select_listWidget->count()<=0)
    {
        return;
    }

    bool flag = false;
    QString selectedListWidgetItem ;
    for(int i=0;i<ui->select_listWidget->count();i++)
    {
        if(ui->select_listWidget->item(i)->isSelected())
        {
            flag = true;
            selectedListWidgetItem = ui->select_listWidget->item(i)->text();
            ui->zhangName_label->setText(selectedListWidgetItem);
            break;
        }
    }
    if(!flag)
        return;

    selectItemButtonClick(selectedListWidgetItem);
    if(ui->select_listWidget->count()>0)
    {
        ui->select_listWidget->item(ui->select_listWidget->count()-1)->setSelected(true);
    }
    if(MainWindowPageFlag == NOVEL_SELECT)
        ui->showListType_label->setText("小说:");
    else
        ui->showListType_label->setText("章节:");
}

void MainWindow::on_saveButton_clicked()
{
    saveTextButtonClick();
    isTextChanged = TEXT_HAVESAVED;//修改标志为已保存
}

void MainWindow::on_backButton_inTextPage_clicked()
{
    backItemButtonClick();
    //设置被选中项获得列表焦点,从TEXT_PAGE页面后退只有到章节选择页面
    //焦点文本为 selectedNovelName
    for(int i=0;i<ui->select_listWidget->count();i++)
    {
        if(ui->select_listWidget->item(i)->text() == selectedZhangName)
        {
            ui->select_listWidget->setCurrentItem(ui->select_listWidget->item(i));
            break;
        }
    }

}

void MainWindow::on_backButton_clicked()
{
    backItemButtonClick();
    //设置被选中项获得列表焦点
    //焦点文本为 selectedNovelName
    for(int i=0;i<ui->select_listWidget->count();i++)
    {
        if(ui->select_listWidget->item(i)->text() == selectedNovelName)
        {
            ui->select_listWidget->setCurrentItem(ui->select_listWidget->item(i));
            break;
        }
    }
     ui->showListType_label->setText("小说:");
}

void MainWindow::on_textEdit_textChanged()
{
    isTextChanged = TEXT_NOSAVED;//修改标志为未保存
    ui->textNumber_label->setText("字数："+QString::number(ui->textEdit->toPlainText().length()));
}

void MainWindow::on_select_listWidget_itemClicked(QListWidgetItem *item)
{
    if(MainWindowPageFlag == NOVEL_SELECT)
    {
        selectedNovelName = item->text();
    }
    else
    {
        selectedZhangName = item->text();
        selectedTextFileName = getTextNameByXml(selectedNovelName,selectedZhangName);
    }
}

void MainWindow::on_textSize_spinBox_valueChanged(int arg1)
{
    //设置改变文本窗体字体大小
    QFont font = ui->textEdit->font();
    font.setPointSize(arg1);
    ui->textEdit->setFont(font);
}

void MainWindow::on_initAll_action_triggered(bool checked)
{
    if(!checked)
    {
        int i = QMessageBox::warning(this,"警告","这么做将会清空你的所有数据\n你确定要这么做吗？"
                              ,QMessageBox::Ok|QMessageBox::Default,QMessageBox::Cancel|QMessageBox::Escape);
        if(i == QMessageBox::Ok)
            ClearAll();
    }
}

void MainWindow::on_daBao_action_triggered(bool checked)
{
    if(!checked)
    {
        QStringList novelNameList = getNovelList(getXmlDir());
        if(novelNameList.length()<=0)
            return;
        bool ok = false;
        QStringList filesNameList;
        QString novelName = QInputDialog::getItem(this, tr("打包"),
                                              tr("请选择待打包的小说："), novelNameList, 0, false,&ok);
        if(ok)
        {
            //打包程序
            QXmlStreamReader reader;
            QFile f(getXmlDir());
            f.open(QIODevice::ReadOnly);
            reader.setDevice(&f);
            //遍历xml文件找到所有与小说名匹配的text文件的文件名
            while(!reader.atEnd())
            {
                if(reader.isStartElement()&&reader.name() == "text")
                {
                    if(reader.attributes().value("novel_name").toString() == novelName)
                    {
                        filesNameList.append(reader.attributes().value("file_name").toString());
                    }
                }
                reader.readNext();
            }
            f.close();

            QFile textFile(getTextDir()+"/FinishNovel/"+novelName+".txt");//打包目录

            QDir textdir(getTextDir());
            textdir.mkdir("FinishNovel");
            QFile textListItemFile(getTextDir()+"/"+filesNameList.at(0));//待打包文件，首先写入第一章
            textListItemFile.open(QIODevice::ReadOnly);
            QTextStream in(&textListItemFile);
            QString text = in.readAll();
            textListItemFile.close();
            textFile.open(QIODevice::WriteOnly);
            textFile.write(text.toUtf8());
            textFile.close();
            for(int i=1;i<filesNameList.length();i++)//循环追加文本
            {
                textListItemFile.setFileName(getTextDir()+"/"+filesNameList.at(i));
                textListItemFile.open(QIODevice::ReadOnly);
                QTextStream stream(&textListItemFile);
                text = stream.readAll();
                textListItemFile.close();
                textFile.open(QIODevice::Append);
                textFile.write("\n");
                textFile.write(text.toUtf8());//追加文本
                textFile.close();
            }
        }
    }
}



QStringList MainWindow::getStyleNameList()
{
    QStringList list;
    QFile f(":/Style.xml");
    QXmlStreamReader reader;
    f.open(QIODevice::ReadOnly);
    reader.setDevice(&f);
    while (!reader.atEnd()) {
        if(reader.isStartElement()&&reader.name()=="style")
        {
            list.append(reader.attributes().value("style_name").toString());
        }
        reader.readNext();
    }
    f.close();
    return list;
}

QString MainWindow::getStyleFileName(QString styleName)
{
    QFile f(":/Style.xml");
    QXmlStreamReader reader;
    f.open(QIODevice::ReadOnly);
    reader.setDevice(&f);

    QString fileName;

    while (!reader.atEnd()) {
        if(reader.isStartElement()&&reader.name()=="style"&&reader.attributes().value("style_name").toString() == styleName)
        {
            fileName = reader.attributes().value("file_name").toString();
            break;
        }
        reader.readNext();
    }
    f.close();
    return fileName;
}


int MainWindow::setStyleByCSS(QString styleCSSFileName)
{
    QString path=":/"+styleCSSFileName;
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    this->setStyleSheet(f.readAll());
    f.close();
    return 0;
}


void MainWindow::on_changeStyle_action_triggered(bool checked)
{
    if(!checked)
    {
        bool ok;
        QString styleName = QInputDialog::getItem(this,tr("更改样式"),tr("请选择样式："),getStyleNameList(),0,false,&ok);
        if(ok){
            setStyleByCSS(getStyleFileName(styleName));
        }
    }
}
