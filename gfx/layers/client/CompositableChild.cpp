/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CompositableChild.h"
#include "CompositableClient.h"

namespace mozilla {
namespace layers {

/* static */ PCompositableChild*
CompositableChild::CreateActor()
{
  CompositableChild* child = new CompositableChild();
  child->AddRef();
  return child;
}

/* static */ void
CompositableChild::DestroyActor(PCompositableChild* aChild)
{
  static_cast<CompositableChild*>(aChild)->Release();
}

CompositableChild::CompositableChild()
 : mCompositableClient(nullptr),
   mCanSend(true)
{
}

CompositableChild::~CompositableChild()
{
}

bool
CompositableChild::IsConnected() const
{
  return mCompositableClient && mCanSend;
}

void
CompositableChild::Init(CompositableClient* aCompositable)
{
  mCompositableClient = aCompositable;
}

void
CompositableChild::RevokeCompositableClient()
{
  mCompositableClient = nullptr;
}

RefPtr<CompositableClient>
CompositableChild::GetCompositableClient()
{
  return mCompositableClient;
}

void
CompositableChild::ActorDestroy(ActorDestroyReason)
{
  MOZ_ASSERT(NS_IsMainThread());

  mCanSend = false;

  if (mCompositableClient) {
    mCompositableClient->mCompositableChild = nullptr;
    mCompositableClient = nullptr;
  }
}

/* static */ PCompositableChild*
AsyncCompositableChild::CreateActor(uint64_t aAsyncID)
{
  AsyncCompositableChild* child = new AsyncCompositableChild(aAsyncID);
  child->AddRef();
  return child;
}

AsyncCompositableChild::AsyncCompositableChild(uint64_t aAsyncID)
 : mLock("AsyncCompositableChild.mLock"),
   mAsyncID(aAsyncID)
{
}

AsyncCompositableChild::~AsyncCompositableChild()
{
}

void
AsyncCompositableChild::ActorDestroy(ActorDestroyReason)
{
  mCanSend = false;

  // We do not revoke CompositableClient::mCompositableChild here, since that
  // could race with the main thread.
  RevokeCompositableClient();
}

void
AsyncCompositableChild::RevokeCompositableClient()
{
  MutexAutoLock lock(mLock);
  mCompositableClient = nullptr;
}

RefPtr<CompositableClient>
AsyncCompositableChild::GetCompositableClient()
{
  MutexAutoLock lock(mLock);
  return CompositableChild::GetCompositableClient();
}

} // namespace layers
} // namespace mozilla
