// Copyright (c) 2015 The EmpireCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "votingtablemodel.h"

#include "guiutil.h"
#include "walletmodel.h"

#include "wallet.h"
#include "base58.h"

#include <QFont>

const QString VotingTableModel::Send = "S";
/* const QString VotingTableModel::Receive = "R"; */

struct VotingTableEntry
{
    QString label;
    QString address;
    QString nation;

    VotingTableEntry() {}
    VotingTableEntry(const QString &label, const QString &address,
                     const QString &nation) :
        label(label), address(address), nation(nation) {}
};

struct VotingTableEntryLessThan
{
    bool operator()(const VotingTableEntry &a, const VotingTableEntry &b) const
    {
        return (a.address < b.address);
    }
    bool operator()(const VotingTableEntry &a, const QString &b) const
    {
        return a.address < b;
    }
    bool operator()(const QString &a, const VotingTableEntry &b) const
    {
        return a < b.address;
    }
};

// Private implementation
class VotingTablePriv
{
public:
    CWallet *wallet;
    QList<VotingTableEntry> cachedVotingTable;
    VotingTableModel *parent;

    VotingTablePriv(CWallet *wallet, VotingTableModel *parent):
        wallet(wallet), parent(parent) {}

    void refreshVotingTable()
    {
        cachedVotingTable.clear();
        {
            LOCK(wallet->cs_wallet);
            typedef std::pair<std::string, std::string> StringPair;
            BOOST_FOREACH(const PAIRTYPE(CTxDestination, StringPair)& item, wallet->mapVotingAddressBook)
            {
                const CEmpireCoinAddress& address = item.first;
                const std::string& strName = item.second.first;
                const std::string& nation = item.second.second;
                cachedVotingTable.append(VotingTableEntry(QString::fromStdString(strName),
                                                          QString::fromStdString(address.ToString()),
                                                          QString::fromStdString(nation)));
            }
        }
        // qLowerBound() and qUpperBound() require our cachedVotingTable list to be sorted in asc order
        qSort(cachedVotingTable.begin(), cachedVotingTable.end(), VotingTableEntryLessThan());
    }

    void updateEntry(const QString &address, const QString &label, const QString &nation, int status)
    {
        // Find address / label in model
        QList<VotingTableEntry>::iterator lower = qLowerBound(
            cachedVotingTable.begin(), cachedVotingTable.end(), address, VotingTableEntryLessThan());
        QList<VotingTableEntry>::iterator upper = qUpperBound(
            cachedVotingTable.begin(), cachedVotingTable.end(), address, VotingTableEntryLessThan());
        int lowerIndex = (lower - cachedVotingTable.begin());
        int upperIndex = (upper - cachedVotingTable.begin());
        bool inModel = (lower != upper);

        switch(status)
        {
        case CT_NEW:
            if(inModel)
            {
                OutputDebugStringF("Warning: VotingTablePriv::updateEntry: Got CT_NOW, but entry is already in model\n");
                break;
            }
            parent->beginInsertRows(QModelIndex(), lowerIndex, lowerIndex);
            cachedVotingTable.insert(lowerIndex, VotingTableEntry(label, address, nation));
            parent->endInsertRows();
            break;
        case CT_UPDATED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: VotingTablePriv::updateEntry: Got CT_UPDATED, but entry is not in model\n");
                break;
            }
            lower->label = label;
            lower->nation = nation;
            parent->emitDataChanged(lowerIndex);
            break;
        case CT_DELETED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: VotingTablePriv::updateEntry: Got CT_DELETED, but entry is not in model\n");
                break;
            }
            parent->beginRemoveRows(QModelIndex(), lowerIndex, upperIndex-1);
            cachedVotingTable.erase(lower, upper);
            parent->endRemoveRows();
            break;
        }
    }

    int size()
    {
        return cachedVotingTable.size();
    }

    VotingTableEntry *index(int idx)
    {
        if(idx >= 0 && idx < cachedVotingTable.size())
        {
            return &cachedVotingTable[idx];
        }
        else
        {
            return 0;
        }
    }
};

VotingTableModel::VotingTableModel(CWallet *wallet, WalletModel *parent) :
    QAbstractTableModel(parent),walletModel(parent),wallet(wallet),priv(0)
{
    columns << tr("Nation") << tr("Address") << tr("Label");
    priv = new VotingTablePriv(wallet, this);
    priv->refreshVotingTable();
}

VotingTableModel::~VotingTableModel()
{
    delete priv;
}

int VotingTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int VotingTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

QVariant VotingTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    VotingTableEntry *rec = static_cast<VotingTableEntry*>(index.internalPointer());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
            case Nation:
            {
                if(rec->nation.isEmpty() && role == Qt::DisplayRole)
                    return tr("A totally independent region");
                else
                    return rec->nation;
                break;
            }
            case Address:
            {
                return rec->address;
            }
            case Label:
            {
                if(rec->label.isEmpty() && role == Qt::DisplayRole)
                    return tr("[empty]");
                else
                    return rec->label;
                break;
            }
        }
    }
    else if (role == Qt::FontRole)
    {
        QFont font;
        if(index.column() == Address)
        {
            font = GUIUtil::EmpireCoinAddressFont();
        }
        return font;
    }
    return QVariant();
}

bool VotingTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;
    VotingTableEntry *rec = static_cast<VotingTableEntry*>(index.internalPointer());

    editStatus = OK;

    if(role == Qt::EditRole)
    {
        LOCK(wallet->cs_wallet); /* For SetVotingAddressBook / DelVotingAddressBook */
        CTxDestination curAddress = CEmpireCoinAddress(rec->address.toStdString()).Get();
        if(index.column() == Label)
        {
            // Do nothing, if old label == new label
            if(rec->label == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
            wallet->SetVotingAddressBookName(curAddress, value.toString().toStdString(), "");
        } else if(index.column() == Address) {
            CTxDestination newAddress = CEmpireCoinAddress(value.toString().toStdString()).Get();
            // Refuse to set invalid address, set error status and return false
            if(boost::get<CNoDestination>(&newAddress))
            {
                editStatus = INVALID_ADDRESS;
                return false;
            }
            // Do nothing, if old address == new address
            else if(newAddress == curAddress)
            {
                editStatus = NO_CHANGES;
                return false;
            }
            // Check for duplicate addresses to prevent accidental deletion of addresses, if you try
            // to paste an existing address over another address (with a different label)
            else if(wallet->mapVotingAddressBook.count(newAddress))
            {
                editStatus = DUPLICATE_ADDRESS;
                return false;
            }
        }
        return true;
    }
    return false;
}

QVariant VotingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return columns[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags VotingTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return 0;

    Qt::ItemFlags retval = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    /* VotingTableEntry *rec = static_cast<VotingTableEntry*>(index.internalPointer()); */
    /* // Can edit address and label for sending addresses, */
    /* // and only label for receiving addresses. */
    /* if(rec->type == VotingTableEntry::Sending || */
    /*   (rec->type == VotingTableEntry::Receiving && index.column()==Label)) */
    /* { */
    /*     retval |= Qt::ItemIsEditable; */
    /* } */
    return retval;
}

QModelIndex VotingTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    VotingTableEntry *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}

void VotingTableModel::updateEntry(const QString &address, const QString &label, const QString &nation, int status)
{
    if (walletModel->validateVotingAddress(address))
    {
        // Update voting address book model from EmpireCoin core
        priv->updateEntry(address, label, nation, status);
    }
}

QString VotingTableModel::addRow(const QString &label, const QString &address, const QString &nation)
{
    std::string strLabel = label.toStdString();
    std::string strAddress = address.toStdString();
    std::string strNation = nation.toStdString();

    editStatus = OK;

    if (!walletModel->validateVotingAddress(address))
    {
        editStatus = INVALID_ADDRESS;
        return QString();
    }

    // Check for duplicate addresses (in this case, it SHOULD be a
    // duplicate since only the label is being updated, the address must exist already)
    {
        LOCK(wallet->cs_wallet);
        if(wallet->mapVotingAddressBook.count(CEmpireCoinAddress(strAddress).Get()))
        {
            // Update entry
            {
                LOCK(wallet->cs_wallet);
                wallet->SetVotingAddressBookName(CEmpireCoinAddress(strAddress).Get(), strLabel, strNation);
            }
            // Update voting address book model
            priv->updateEntry(QString::fromStdString(strAddress), QString::fromStdString(strLabel),
                              QString::fromStdString(strNation), (int)CT_UPDATED);
            return QString::fromStdString(strAddress);
        }
    }
    return QString();
}

bool VotingTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    VotingTableEntry *rec = priv->index(row);
    // NOTE: we allow removal as a replacement (receive address is still valid after?!?!)
    // visually only.  One is shown at at time ... maybe? FIXME

    /* if(count != 1 || !rec || rec->type == VotingTableEntry::Receiving) */
    /* { */
    /*     // Can only remove one row at a time, and cannot remove rows not in model. */
    /*     // Also refuse to remove receiving addresses. */
    /*     return false; */
    /* } */
    {
        LOCK(wallet->cs_wallet);
        wallet->DelAddressBookName(CEmpireCoinAddress(rec->address.toStdString()).Get());
    }
    return true;
}

/* Look up label and nation for address in address book, if not found
 * return a pair of empty strings.
 */
std::pair<QString, QString> VotingTableModel::labelsForAddress(const QString &address) const
{
    {
        LOCK(wallet->cs_wallet);
        CEmpireCoinAddress address_parsed(address.toStdString());
        std::map<CTxDestination, std::pair<std::string, std::string> >::iterator mi =
            wallet->mapVotingAddressBook.find(address_parsed.Get());
        if (mi != wallet->mapVotingAddressBook.end())
        {
            return std::make_pair(QString::fromStdString(mi->second.first),
                                  QString::fromStdString(mi->second.second));
        }
    }
    return std::make_pair(QString(), QString());
}

int VotingTableModel::lookupAddress(const QString &address) const
{
    QModelIndexList lst = match(index(0, Address, QModelIndex()),
                                Qt::EditRole, address, 1, Qt::MatchExactly);
    if(lst.isEmpty())
    {
        return -1;
    }
    else
    {
        return lst.at(0).row();
    }
}

void VotingTableModel::emitDataChanged(int idx)
{
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, columns.length()-1, QModelIndex()));
}
