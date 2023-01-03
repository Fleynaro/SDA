import { Offset } from 'sda-core';
import { binSearch } from './common';

export class Interval {
  public start: number;
  public end: number;

  constructor(start: number, end: number) {
    this.start = start;
    this.end = end;
  }

  public get length(): number {
    return this.end - this.start;
  }

  public intersects(interval: Interval): boolean {
    return this.start <= interval.end && this.end >= interval.start;
  }

  public strictlyIntersects(interval: Interval): boolean {
    return this.start < interval.end && this.end > interval.start;
  }
}

export class Jump {
  public offset: Offset;
  public targetOffset: Offset;

  constructor(offset: Offset, targetOffset: Offset) {
    this.offset = offset;
    this.targetOffset = targetOffset;
  }

  public get interval(): Interval {
    return this.offset <= this.targetOffset
      ? new Interval(this.offset, this.targetOffset)
      : new Interval(this.targetOffset, this.offset);
  }
}

export class JumpManager {
  private jumps: Jump[][];

  constructor() {
    this.jumps = [[]];
  }

  public addJump(jump: Jump) {
    let layerLevel = 0;
    // find the first layer where there are no jumps intersecting with the new jump
    while (layerLevel < 1000) {
      const jumpsOnLevel = this.jumps[layerLevel];
      const nearestRightJumpIdx = this.findNearestRightJumpIdx(jumpsOnLevel, jump.interval.start);
      const [foundIntersectedJumps, foundIntersectedJumpsIdx] = this.findJumpsInInterval_(
        jumpsOnLevel,
        jump.interval,
        nearestRightJumpIdx,
        true,
      );
      if (foundIntersectedJumps.length === 0) {
        if (nearestRightJumpIdx !== -1) {
          let insertBeforeIdx = nearestRightJumpIdx;
          if (jump.interval.start === jumpsOnLevel[insertBeforeIdx].interval.end) {
            insertBeforeIdx++;
          }
          jumpsOnLevel.splice(insertBeforeIdx, 0, jump);
        } else {
          jumpsOnLevel.push(jump);
        }
        break;
      } else if (foundIntersectedJumps.length === 1) {
        // larger jumps should be on the higher layers to minimize the number of intersections
        const foundJump = foundIntersectedJumps[0];
        const foundJumpIdx = foundIntersectedJumpsIdx[0];
        if (foundJump.interval.length > jump.interval.length) {
          [jumpsOnLevel[foundJumpIdx], jump] = [jump, foundJump];
        }
      }
      layerLevel++;
      if (layerLevel === this.jumps.length) {
        this.jumps.push([]);
      }
    }
  }

  // public addJumps(jumps: Jump[]) {
  //   const sortedJumps = jumps.sort((a, b) => a.interval.length - b.interval.length);
  //   for (const jump of sortedJumps) {
  //     this.addJump(jump);
  //   }
  // }

  public findJumpsInInterval(interval: Interval): Jump[][] {
    const foundJumps: Jump[][] = [];
    for (const jumpsOnLevel of this.jumps) {
      const nearestRightJumpIdx = this.findNearestRightJumpIdx(jumpsOnLevel, interval.start);
      const [foundJumpsOnLevel, _] = this.findJumpsInInterval_(
        jumpsOnLevel,
        interval,
        nearestRightJumpIdx,
        false,
      );
      foundJumps.push(foundJumpsOnLevel);
    }
    return foundJumps;
  }

  private findNearestRightJumpIdx(jumps: Jump[], offset: Offset): number {
    return binSearch(jumps, (mid) => {
      const jump = jumps[mid];
      if (mid > 0) {
        const prevJump = jumps[mid - 1];
        if (prevJump.interval.end < offset && offset <= jump.interval.end) return 0;
      } else {
        if (offset <= jump.interval.end) return 0;
      }
      return jump.interval.end - offset;
    });
  }

  private findJumpsInInterval_(
    jumps: Jump[],
    interval: Interval,
    nearestRightJumpIdx: number,
    strictIntersection: boolean,
  ): [Jump[], number[]] {
    const foundJumps: Jump[] = [];
    const foundJumpsIdx: number[] = [];
    if (nearestRightJumpIdx !== -1) {
      for (let i = nearestRightJumpIdx; i < jumps.length; i++) {
        const jump = jumps[i];
        if (
          strictIntersection
            ? jump.interval.strictlyIntersects(interval)
            : jump.interval.intersects(interval)
        ) {
          foundJumps.push(jump);
          foundJumpsIdx.push(i);
        } else {
          break;
        }
      }
    }
    return [foundJumps, foundJumpsIdx];
  }
}

// eslint-disable-next-line @typescript-eslint/no-unused-vars
function Test() {
  const jumps = new JumpManager();
  jumps.addJump(new Jump(0, 10));
  jumps.addJump(new Jump(15, 20));
  jumps.addJump(new Jump(10, 15));
  jumps.addJump(new Jump(16, 18));
  jumps.addJump(new Jump(16, 18));
  // jumps.addJumps([
  //   new Jump(0, 10),
  //   new Jump(15, 20),
  //   new Jump(10, 15),
  //   new Jump(16, 18),
  //   new Jump(16, 18),
  // ]);

  const foundJumps = jumps.findJumpsInInterval(new Interval(14, 17));
  console.log(foundJumps);
}

//Test();
