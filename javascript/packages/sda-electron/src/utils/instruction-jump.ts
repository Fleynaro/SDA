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

  public equals(jump: Jump): boolean {
    return this.offset === jump.offset && this.targetOffset === jump.targetOffset;
  }
}

export class JumpManager {
  private jumps: Jump[][]; // [layer][jump]

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

  public removeJump(jump: Jump) {
    const jumpsToAddBack: Jump[] = [];
    let startRemvoingAll = false;
    for (const jumpsOnLevel of this.jumps) {
      const nearestRightJumpIdx = this.findNearestRightJumpIdx(jumpsOnLevel, jump.interval.start);
      const [foundJumpsOnLevel, foundJumpsOnLevelIdx] = this.findJumpsInInterval_(
        jumpsOnLevel,
        jump.interval,
        nearestRightJumpIdx,
        true,
      );
      if (!startRemvoingAll) {
        const jumpToRemoveRelIdx = foundJumpsOnLevel.findIndex((j) => j.equals(jump));
        if (jumpToRemoveRelIdx !== -1) {
          const jumpToRemoveIdx = foundJumpsOnLevelIdx[jumpToRemoveRelIdx];
          jumpsOnLevel.splice(jumpToRemoveIdx, 1);
          // remove all intersected jumps on above layers and then add them back (to reach right arrangement of jumps)
          startRemvoingAll = true;
        }
      } else {
        const sortedFoundJumpsOnLevelIdx = foundJumpsOnLevelIdx.sort((a, b) => b - a);
        for (const idx of sortedFoundJumpsOnLevelIdx) {
          jumpsOnLevel.splice(idx, 1);
        }
        jumpsToAddBack.push(...foundJumpsOnLevel);
      }
    }
    // add back jumps
    for (const jumpToAddBack of jumpsToAddBack) {
      this.addJump(jumpToAddBack);
    }
    // remove empty layers
    let layerLevel = this.jumps.length - 1;
    while (layerLevel >= 0) {
      if (this.jumps[layerLevel].length === 0) {
        this.jumps.splice(layerLevel, 1);
      }
      layerLevel--;
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
          if (!strictIntersection || i > nearestRightJumpIdx) {
            break;
          }
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
  jumps.addJump(new Jump(17, 18));
  {
    const foundJumps = jumps.findJumpsInInterval(new Interval(14, 17));
    console.log(foundJumps);
    /* !!!shown interval [14, 17]!!!
    [
      [
        Jump { offset: 10, targetOffset: 15 },
        Jump { offset: 17, targetOffset: 18 }
      ],
      [ Jump { offset: 16, targetOffset: 18 } ],
      [ Jump { offset: 15, targetOffset: 20 } ]
    ]
    */
  }
  // remove jump
  jumps.removeJump(new Jump(17, 18));
  {
    const foundJumps = jumps.findJumpsInInterval(new Interval(14, 17));
    console.log(foundJumps);
    /* !!!shown interval [14, 17]!!!
    [
      [
        Jump { offset: 10, targetOffset: 15 },
        Jump { offset: 16, targetOffset: 18 }
      ],
      [ Jump { offset: 15, targetOffset: 20 } ]
    ]
    */
  }
}

Test();
