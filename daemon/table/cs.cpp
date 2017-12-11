/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cs.hpp"
#include "core/algorithm.hpp"
#include "core/asserts.hpp"
#include "core/logger.hpp"
#include <ndn-cxx/lp/tags.hpp>

namespace nfd {
namespace cs {

NFD_ASSERT_FORWARD_ITERATOR(Cs::const_iterator);

NFD_LOG_INIT("ContentStore");

unique_ptr<Policy>
makeDefaultPolicy()
{
  const std::string DEFAULT_POLICY = "priority_fifo";
  return Policy::create(DEFAULT_POLICY);
}

Cs::Cs(size_t nMaxPackets)
{
  this->setPolicyImpl(makeDefaultPolicy());
  ptm = PTManager::getInstance();
  m_policy->setLimit(nMaxPackets);
}

void
Cs::setLimit(size_t nMaxPackets)
{
  m_policy->setLimit(nMaxPackets);
}

size_t
Cs::getLimit() const
{
  return m_policy->getLimit();
}

void
Cs::setPolicy(unique_ptr<Policy> policy)
{
  BOOST_ASSERT(policy != nullptr);
  BOOST_ASSERT(m_policy != nullptr);
  size_t limit = m_policy->getLimit();
  this->setPolicyImpl(std::move(policy));
  m_policy->setLimit(limit);
}

void
Cs::insert(const Data& data, bool isUnsolicited)
{
  NFD_LOG_DEBUG("insert " << data.getName());

  if (m_policy->getLimit() == 0) {
    // shortcut for disabled CS
    return;
  }

  // recognize CachePolicy
  shared_ptr<lp::CachePolicyTag> tag = data.getTag<lp::CachePolicyTag>();
  if (tag != nullptr) {
    lp::CachePolicyType policy = tag->get().getPolicy();
    if (policy == lp::CachePolicyType::NO_CACHE) {
      return;
    }
  }

  bool isNewEntry = false;
  iterator it;
  // use .insert because gcc46 does not support .emplace
  std::tie(it, isNewEntry) = m_table.insert(EntryImpl(data.shared_from_this(), isUnsolicited));
  EntryImpl& entry = const_cast<EntryImpl&>(*it);

  entry.updateStaleTime();

  if (!isNewEntry) { // existing entry
    // XXX This doesn't forbid unsolicited Data from refreshing a solicited entry.
    if (entry.isUnsolicited() && !isUnsolicited) {
      entry.unsetUnsolicited();
    }

    m_policy->afterRefresh(it);
  }
  else {
    m_policy->afterInsert(it);
  }
}

void
Cs::find(const Interest& interest,
         const HitCallback& hitCallback,
         const MissCallback& missCallback) const
{
  BOOST_ASSERT(static_cast<bool>(hitCallback));
  BOOST_ASSERT(static_cast<bool>(missCallback));

  const Name& prefix = interest.getName();
  bool isRightmost = interest.getChildSelector() == 1;
  NFD_LOG_DEBUG("find " << prefix << (isRightmost ? " R" : " L"));


// NEW CHANGE
  // ignore reserved localhost command.
  if(!Name("/localhost").isPrefixOf(prefix)){
    //ptm->print_publist();
  // check if it is a private request.
    if(ptm->amIPrivate()){

      if(!ptm->isPublic(prefix)){
        //std::cout << "I'm private request, and my name is not in PubList" << std::endl;
        std::string myNonce = ptm->getMyNonce();
        //std::cout<<"I'm private request, my nonce is "<< myNonce << std::endl;

        // If it's a private request, check if there are peer pentry with the same name in the ptable.
        // if there is any peer pentries, this request should be delayed for once.
        // since if it doesn't, the privacy of peer is leaked.
        if (ptm->peer_check(prefix, myNonce)) {
          //std::cout <<"I found myself is in ptable with peer" << std::endl;

          // Then check if i have been delayed once for any peers.
          // if yes, that means this time the request does not have to delay anymore.
          // since the previous request could be cached by at least once.
          if (ptm->hasDelayed(prefix, myNonce)) {
            //std::cout <<"but i have delayed for others, proceed as normal" << std::endl;
          } 

          // If not, then this request would have to delay once to protect the privacy of peers.
          else {
            //std::cout <<"but i'm first time here, delay once" << std::endl;
            //ptm->resetLastPair();
            ptm->setDelayed(prefix, myNonce, true);
            ptm->dequeue_pair();
            missCallback(interest);
            return;
          }
        } 

        // If there is no peer pentries with the given name.
        // this request becomes the first one, and does not have to consider for others.
        else {
          //std::cout <<"I found myself is peerless in ptable, proceed as normal" << std::endl;
          ptm->setDelayed(prefix, myNonce, true);
        }

      } else {
        //std::cout << "I'm private request, but my name is in PubList, proceed as normal" << std::endl;
      }
    } 

    // if it's not a private request, then any pentries in ptable should be invalidated.
    // The invalidation marks this name as being publicly accessed, so no delay is require anymore.
    else {
      //std::cout<<interest.getName()<<" is not private" << std::endl;
      if (ptm->isNamePrivate(prefix)){
        //std::cout<<"There are private entry of my name in ptable, delay once, invalidate all" << std::endl;
        ptm->invalidate_all(prefix);
        //ptm->print_table();
        //ptm->resetLastPair();
        ptm->dequeue_pair();
        missCallback(interest);
        return;
      } else {
        //std::cout<<"There is no private entry of my name in ptable, proceed as normal" << std::endl;
      }

      // insert the public request into Publist is not there
      if(!ptm->isPublic(prefix)){
        //std::cout << "My name is not in PubList, add myself to it" << std::endl;
        ptm->publist_insert(prefix.toUri());
      }
      else{
        //std::cout << "My name is in PubList, update timer and proceed" << std::endl;
      }
    }

    //ptm->resetLastPair();
    ptm->dequeue_pair();

  }
  // CHANGE NEW
  
  iterator first = m_table.lower_bound(prefix);
  iterator last = m_table.end();
  if (prefix.size() > 0) {
    last = m_table.lower_bound(prefix.getSuccessor());
  }

  iterator match = last;
  if (isRightmost) {
    match = this->findRightmost(interest, first, last);
  }
  else {
    match = this->findLeftmost(interest, first, last);
  }

  if (match == last) {
    NFD_LOG_DEBUG("  no-match");
    missCallback(interest);
    return;
  }
  NFD_LOG_DEBUG("  matching " << match->getName());
  m_policy->beforeUse(match);
  hitCallback(interest, match->getData());
}

iterator
Cs::findLeftmost(const Interest& interest, iterator first, iterator last) const
{
  return std::find_if(first, last, bind(&cs::EntryImpl::canSatisfy, _1, interest));
}

iterator
Cs::findRightmost(const Interest& interest, iterator first, iterator last) const
{
  // Each loop visits a sub-namespace under a prefix one component longer than Interest Name.
  // If there is a match in that sub-namespace, the leftmost match is returned;
  // otherwise, loop continues.

  size_t interestNameLength = interest.getName().size();
  for (iterator right = last; right != first;) {
    iterator prev = std::prev(right);

    // special case: [first,prev] have exact Names
    if (prev->getName().size() == interestNameLength) {
      NFD_LOG_TRACE("  find-among-exact " << prev->getName());
      iterator matchExact = this->findRightmostAmongExact(interest, first, right);
      return matchExact == right ? last : matchExact;
    }

    Name prefix = prev->getName().getPrefix(interestNameLength + 1);
    iterator left = m_table.lower_bound(prefix);

    // normal case: [left,right) are under one-component-longer prefix
    NFD_LOG_TRACE("  find-under-prefix " << prefix);
    iterator match = this->findLeftmost(interest, left, right);
    if (match != right) {
      return match;
    }
    right = left;
  }
  return last;
}

iterator
Cs::findRightmostAmongExact(const Interest& interest, iterator first, iterator last) const
{
  return find_last_if(first, last, bind(&EntryImpl::canSatisfy, _1, interest));
}

void
Cs::setPolicyImpl(unique_ptr<Policy> policy)
{
  NFD_LOG_DEBUG("set-policy " << policy->getName());
  m_policy = std::move(policy);
  m_beforeEvictConnection = m_policy->beforeEvict.connect([this] (iterator it) {
      m_table.erase(it);
    });

  m_policy->setCs(this);
  BOOST_ASSERT(m_policy->getCs() == this);
}

void
Cs::dump()
{
  NFD_LOG_DEBUG("dump table");
  for (const EntryImpl& entry : m_table) {
    NFD_LOG_TRACE(entry.getFullName());
  }
}

} // namespace cs
} // namespace nfd
