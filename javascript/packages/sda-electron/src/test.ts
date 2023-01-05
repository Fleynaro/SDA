import { ProjectContextCallbacks } from 'sda';
import { Context, AddressSpace, ContextCallbacks, Image } from 'sda-core';

const testContext = (body: () => void, ctx: Context) => {
  const projectCallbacks = ContextCallbacks.Find(
    'Project',
    ctx.callbacks,
  ) as ProjectContextCallbacks;
  projectCallbacks.setChangeEnabled(false);
  projectCallbacks.setTransactionEnabled(false);
  body();
  projectCallbacks.setChangeEnabled(true);
  projectCallbacks.setTransactionEnabled(true);
};

export const createTestObjects = (ctx: Context) =>
  testContext(() => {
    const addressSpace = AddressSpace.New(ctx, 'Test Address Space');
    const testImage = Image.New(addressSpace, 'Test Image');
  }, ctx);
