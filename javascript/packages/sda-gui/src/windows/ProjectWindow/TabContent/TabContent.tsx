import { Box, emphasize, useTheme } from '@mui/material';
import { Resizable } from 're-resizable';
import { ObjectId } from 'sda-electron/api/common';
import { getImageApi } from 'sda-electron/api/image';
import {
  ImageContent,
  ImageContentContextMenu,
  ImageContentProvider,
  ImageContentStyleBridgeConsumer,
  ImageContentStyleBridgeProvider,
  useImageContent,
} from 'components/ImageContent';
import {
  Block,
  Text,
  KonvaFormatTextSelectionBridgeConsumer,
  KonvaFormatTextSelectionBridgeProvider,
  Stage,
  StaticText,
} from 'components/Konva';
import { useContextMenu } from 'components/Menu';
import { useProjectWindowStyles } from '../style';
import Konva from 'konva';
import Button from '@mui/material/Button';
import { useObject } from 'hooks';
import {
  ImageContentBridgeConsumer,
  ImageContentBridgeProvider,
} from 'components/ImageContent/context';
import { Layer } from 'react-konva';
import { useEffect, useState } from 'react';

/*
  Логическая или физическая группировка элементов?
  x = (5 + 5) +
      (2 * 10);
  
  Ответ: логическая! Само выражение должно приходить с \n и \t
  x = (5 + 5) +\n
  \t(2 * 10);

  Компонент будет строиться рекурисвно:
  <Node/><Text onClick={...}>=</Text><Node/><Text>;</Text>

  !оператор присваивания выделяется только по нажатию на =, при этом ; к нему не относится
  !Text - листья дерева

  НО проблема с номерами строк... Но их можно нарисовать отдельно как и стрелки в ImageContent. Надо, чтобы block хранил строки элементов

  Ответ 2: физическая! Нам на фронт приходит линейный список токенов (с токенами \n и \t), а также action, представленные в виде дерева вложенных отрезков ([startTokenIdx, endTokenIdx]).
  При рендеринге мы каждый токен должны сопоставить с отрезком action. Мы можем также выделять текст ([selStartTokenIdx, selEndTokenIdx]),
  ищем в этом выделении, например, все операции, которые имеют связь с инструкцией P-кода и делаем их выделения в ImageContent
  P.S. Отрезков должно быть много, а дерево глубоким - для быстрого поиска
  P.S.2. Можно юзать Jumps, вынести в отдельный пакет sda-common
*/

// - TestGroup и TestItem сделать "невидимыми коробками", которые добавляют физику, но не графику
// - возможность писать текст в виде токенов внутри контейнера. Указывать стили не только для каждого токена, но и в целом для всех в контейнере
// - возможность выбирать направления потока элементов как в flex: row, column
// - (сделано) избавиться от необходимости указывать индексы idx (https://stackoverflow.com/questions/51063086/what-is-the-most-effective-way-to-get-children-at-index-in-react)
// - возможность указывать отступы между элементами (margin) и от краев контейнера (padding)
/*
<TextSelectionArea name="pcode">
  <Group direction="col">
    <Group direction="row" style={{ width: 100, marginTop: 10, padding: 10, fill: 'red', fontSize: 14 }}>
      <Group onClick={() => console.log('varnode)}>
        <Text>edx</Text>
      </Group>
      <Text> = </Text>
      <Text style={{ fill: 'green' }}>COPY</Text>
      <Group onClick={() => console.log('varnode)}>
        <Text>eax</Text>
      </Group>
      // <NewLine />
      // <CustomElement />
    </Group>
    ...
  <Group direction="col">
</TextSelectionArea>
Group - Grid
*/
// ------------------------------

const DecompilerComponent = () => {
  const {
    imageId,
    rowSelection: { selectedRows },
    functions: { goToOffset },
  } = useImageContent();
  const image = useObject(() => getImageApi().getImage(imageId), [imageId]);
  if (!image) return null;
  return (
    <Box>
      {imageId.key} <br />
      Decompiler ({selectedRows.length} rows selected) <br />
      <Button variant="contained" onClick={() => goToOffset(image.entryPointOffset)}>
        Go to begining
      </Button>
    </Box>
  );
};

export interface TabContentProps {
  imageId: ObjectId;
}

export const TabContent = ({ imageId }: TabContentProps) => {
  const theme = useTheme();
  const classes = useProjectWindowStyles();
  const contextMenu = useContextMenu();

  const onContextMenu = (e: Konva.KonvaEventObject<MouseEvent>) => {
    contextMenu.openAtPos(e.evt.clientX, e.evt.clientY);
  };

  const [size, setSize] = useState(0);

  useEffect(() => {
    setInterval(() => {
      setSize((s) => s + 1);
    }, 500);
  }, []);

  // There's a trouble with context exposing into konva. Solve: https://github.com/konvajs/react-konva/issues/188#issuecomment-478302062
  return (
    <ImageContentProvider imageId={imageId}>
      <KonvaFormatTextSelectionBridgeConsumer>
        {(value1) => (
          <ImageContentStyleBridgeConsumer>
            {(value2) => (
              <ImageContentBridgeConsumer>
                {(value3) => (
                  <Stage
                    sx={{ width: '100%', height: '100%', flexGrow: 1 }}
                    onContextMenu={onContextMenu}
                  >
                    <KonvaFormatTextSelectionBridgeProvider value={value1}>
                      <ImageContentStyleBridgeProvider value={value2}>
                        <ImageContentBridgeProvider value={value3}>
                          <ImageContent />
                          {/* <Layer>
                            <TestGroup>
                              {Array.from({ length: 100 }).map((_, idx) => (
                                <TestItem key={idx} width={200} height={5} />
                              ))}
                            </TestGroup>
                          </Layer> */}
                          {/* <Layer>
                            <Block
                              x={200}
                              width={100}
                              flexDir="row"
                              padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
                            >
                              <BlockChildContext.Consumer>
                                {(context) => (
                                  <Rect
                                    width={context?.width || 0}
                                    height={context?.height || 0}
                                    fill="red"
                                  />
                                )}
                              </BlockChildContext.Consumer>
                              <Block width={30} height={10}>
                                <Rect width={30} height={10} fill="green" />
                              </Block>
                              <Block width={30} height={20} margin={{ left: 2, right: 2 }}>
                                <Rect width={30} height={20} fill="green" />
                              </Block>
                              <Block width={30} height={10}>
                                <Rect width={30} height={10} fill="green" />
                              </Block>
                              <Block width={30} height={10}>
                                <Rect width={30} height={10} fill="green" />
                              </Block>
                            </Block>
                          </Layer> */}
                          <Layer>
                            <Block
                              idx={0}
                              x={310}
                              width={400}
                              flexDir="col"
                              fill="blue"
                              //padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
                              textStyle={{ fill: 'red' }}
                            >
                              {Array.from({ length: 3 }).map((_, idx) => (
                                <Block
                                  key={idx}
                                  idx={idx}
                                  width="100%"
                                  margin={{ top: idx === 0 ? 0 : 5 }}
                                  padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
                                  fill="green"
                                  onClick={() => console.log('click', idx)}
                                >
                                  <Block idx={0} width={30} height={10} fill="red" />
                                  <Block
                                    idx={1}
                                    width={30}
                                    height={10}
                                    fill="red"
                                    margin={{ left: 5 }}
                                  />
                                  <Block
                                    idx={2}
                                    width={30}
                                    height={20}
                                    fill="red"
                                    margin={{ left: 5 }}
                                  />
                                  <Block
                                    idx={3}
                                    width="grow"
                                    fill="yellow"
                                    textStyle={{ fontSize: 14 }}
                                  >
                                    <StaticText idx={0} text="Hello " fontSize={15} />
                                    <StaticText idx={1} text="guys!" />
                                  </Block>
                                  <Block idx={4} fill="yellow" textStyle={{ fontSize: 15 }}>
                                    <StaticText idx={0} text="Hello " />
                                    <StaticText idx={1} text="guys!" />
                                  </Block>
                                  {/* <Group>
                                    <Block width={30} height={10} fill="blue" />
                                    <Block width={30} height={10} fill="blue" />
                                  </Group> */}
                                </Block>
                              ))}
                            </Block>
                          </Layer>
                        </ImageContentBridgeProvider>
                      </ImageContentStyleBridgeProvider>
                    </KonvaFormatTextSelectionBridgeProvider>
                  </Stage>
                )}
              </ImageContentBridgeConsumer>
            )}
          </ImageContentStyleBridgeConsumer>
        )}
      </KonvaFormatTextSelectionBridgeConsumer>
      <Resizable
        defaultSize={{
          width: 200,
          height: 'auto',
        }}
        enable={{ left: true }}
        handleClasses={{
          left: classes.resizable,
        }}
      >
        <Box
          sx={{
            backgroundColor: emphasize(theme.palette.background.default, 0.05),
            height: '100%',
          }}
        >
          <DecompilerComponent />
        </Box>
      </Resizable>
      <ImageContentContextMenu {...contextMenu.props} />
    </ImageContentProvider>
  );
};
