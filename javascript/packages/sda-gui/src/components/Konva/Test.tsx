import { Block, StaticText } from 'components/Konva';
import { useEffect, useState } from 'react';

export const Test = () => {
  const [p, setP] = useState(0);

  useEffect(() => {
    setInterval(() => {
      setP((p) => p + 1);
    }, 500);
  }, []);

  return (
    <Block
      idx={0}
      x={310}
      width={400}
      flexDir="col"
      fill="blue"
      //padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
      textStyle={{ fill: 'red' }}
    >
      {Array.from({ length: 30 }).map((_, idx) => (
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
          <Block idx={1} width={30} height={10} fill="red" margin={{ left: 5 }} />
          <Block idx={2} width={30 + p} height={20} fill="red" margin={{ left: 5 }} />
          <Block idx={3} width="grow" fill="yellow" textStyle={{ fontSize: 14 }}>
            <StaticText idx={0} text={`Hello ${idx}`} fontSize={15} />
            <StaticText idx={1} text="guys!" />
          </Block>
          <Block idx={4} fill="yellow" textStyle={{ fontSize: 15 }}>
            <StaticText idx={0} text="Hello " />
            <StaticText idx={1} text="guys!" />
          </Block>
        </Block>
      ))}
    </Block>
  );
};

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
