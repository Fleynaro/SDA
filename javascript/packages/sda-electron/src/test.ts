import { Context, AddressSpace, ContextCallbacks, Image } from 'sda-core';

const testContext = (body: () => void, ctx: Context) => {
  const transactionCallbacks = ContextCallbacks.Find('Transaction', ctx.callbacks);
  const changeChainCallbacks = ContextCallbacks.Find('ChangeChain', ctx.callbacks);
  transactionCallbacks?.setEnabled(false);
  changeChainCallbacks?.setEnabled(false);
  body();
  transactionCallbacks?.setEnabled(true);
  changeChainCallbacks?.setEnabled(true);
};

export const createTestObjects = (ctx: Context) =>
  testContext(() => {
    const addressSpace = AddressSpace.New(ctx, 'Test Address Space');
    //const testImage = Image.New(addressSpace, 'Test Image');
  }, ctx);
