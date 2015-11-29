// Copyright (c) 2015 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "votingpage.h"
#include "ui_votingpage.h"

#include "votingtablemodel.h"
#include "optionsmodel.h"
#include "empirecoingui.h"
#include "editvotingaddressdialog.h"
#include "csvmodelwriter.h"
#include "guiutil.h"

#ifdef USE_QRCODE
#include "qrcodedialog.h"
#endif

#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>

VotingPage::VotingPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VotingPage),
    model(0),
    optionsModel(0)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    ui->newAddress->setIcon(QIcon());
    ui->copyAddress->setIcon(QIcon());
    ui->exportButton->setIcon(QIcon());
#endif

#ifndef USE_QRCODE
    ui->showQRCode->setVisible(false);
#endif

    ui->labelExplanation->setText(tr("Primary Voting Addresses."));

    // Context menu actions
    QAction *copyAddressAction = new QAction(ui->copyAddress->text(), this);
    QAction *copyLabelAction = new QAction(tr("Copy &Label"), this);
    QAction *editAction = new QAction(tr("&Edit"), this);
    QAction *showQRCodeAction = new QAction(ui->showQRCode->text(), this);

    ui->newAddress->setEnabled(false);
    ui->copyAddress->setEnabled(false);
    ui->showQRCode->setEnabled(false);

    // Build context menu
    contextMenu = new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(editAction);
    contextMenu->addSeparator();
#ifdef USE_QRCODE
    contextMenu->addAction(showQRCodeAction);
#endif

    // Connect signals for context menu actions
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(on_copyAddress_clicked()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(onCopyLabelAction()));
    connect(editAction, SIGNAL(triggered()), this, SLOT(onEditAction()));
    connect(showQRCodeAction, SIGNAL(triggered()), this, SLOT(on_showQRCode_clicked()));

    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));
}

VotingPage::~VotingPage()
{
    delete ui;
}

void VotingPage::setModel(VotingTableModel *model)
{
    this->model = model;
    if(!model)
        return;

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->tableView->setModel(proxyModel);
    ui->tableView->sortByColumn(1, Qt::AscendingOrder);

    // Set column widths
#if QT_VERSION < 0x050000
    ui->tableView->horizontalHeader()->setResizeMode(VotingTableModel::Nation, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setResizeMode(VotingTableModel::Address, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setResizeMode(VotingTableModel::Label, QHeaderView::ResizeToContents);
#else
    ui->tableView->horizontalHeader()->setSectionResizeMode(VotingTableModel::Nation, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(VotingTableModel::Address, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(VotingTableModel::Label, QHeaderView::ResizeToContents);
#endif

    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged()));

    // Select row for newly created address
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(selectNewAddress(QModelIndex,int,int)));

    selectionChanged();
}

void VotingPage::setOptionsModel(OptionsModel *optionsModel)
{
    this->optionsModel = optionsModel;
}

void VotingPage::on_copyAddress_clicked()
{
    GUIUtil::copyEntryData(ui->tableView, VotingTableModel::Address);
}

void VotingPage::onCopyLabelAction()
{
    GUIUtil::copyEntryData(ui->tableView, VotingTableModel::Label);
}

void VotingPage::onEditAction()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
    if(indexes.isEmpty())
        return;

    EditVotingAddressDialog dlg;
    dlg.setModel(model);
    QModelIndex origIndex = proxyModel->mapToSource(indexes.at(0));
    dlg.loadRow(origIndex.row());
    dlg.exec();
}

void VotingPage::on_newAddress_clicked()
{
    if(!model)
        return;

    EditVotingAddressDialog dlg;
    dlg.setModel(model);
    if(dlg.exec())
    {
        newAddressToSelect = dlg.getAddress();
    }
}

void VotingPage::selectionChanged()
{
    // Set button states based on selected tab and selection
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;

    if(table->selectionModel()->hasSelection())
    {
        ui->newAddress->setEnabled(true);
        ui->copyAddress->setEnabled(true);
        ui->showQRCode->setEnabled(true);
    }
}

void VotingPage::done(int retval)
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel() || !table->model())
        return;
    // When this is a tab/widget and not a model dialog, ignore "done"
    /* if(mode == ForEditing) */
    /*     return; */

    // Figure out which address was selected, and return it
    QModelIndexList indexes = table->selectionModel()->selectedRows(VotingTableModel::Address);

    foreach (QModelIndex index, indexes)
    {
        QVariant address = table->model()->data(index);
        returnValue = address.toString();
    }

    if(returnValue.isEmpty())
    {
        // If no address entry selected, return rejected
        retval = Rejected;
    }

    QDialog::done(retval);
}

void VotingPage::on_exportButton_clicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this,
            tr("Export Address Book Data"), QString(),
            tr("Comma separated file (*.csv)"));

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(proxyModel);
    writer.addColumn("Nation", VotingTableModel::Nation, Qt::EditRole);
    writer.addColumn("Address", VotingTableModel::Address, Qt::EditRole);
    writer.addColumn("Label", VotingTableModel::Label, Qt::EditRole);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void VotingPage::on_showQRCode_clicked()
{
#ifdef USE_QRCODE
    QTableView *table = ui->tableView;
    QModelIndexList indexes = table->selectionModel()->selectedRows(VotingTableModel::Address);

    foreach (QModelIndex index, indexes)
    {
        QString address = index.data().toString();
        QString label = index.sibling(index.row(), 0).data(Qt::EditRole).toString();

        QRCodeDialog *dialog = new QRCodeDialog(address, label, true/* tab == ReceivingTab */, this);
        dialog->setModel(optionsModel);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
#endif
}

void VotingPage::contextualMenu(const QPoint &point)
{
    QModelIndex index = ui->tableView->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void VotingPage::selectNewAddress(const QModelIndex &parent, int begin, int /*end*/)
{
    QModelIndex idx = proxyModel->mapFromSource(model->index(begin, VotingTableModel::Address, parent));
    if(idx.isValid() && (idx.data(Qt::EditRole).toString() == newAddressToSelect))
    {
        // Select row of newly created address, once
        ui->tableView->setFocus();
        ui->tableView->selectRow(idx.row());
        newAddressToSelect.clear();
    }
}
