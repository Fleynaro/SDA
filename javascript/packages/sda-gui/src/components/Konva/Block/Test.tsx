import { Block, StaticTextBlock, TextBlock } from 'components/Konva';

export const Test1 = () => {
  // const [p, setP] = useState(0);

  // useEffect(() => {
  //   setInterval(() => {
  //     setP((p) => p + 1);
  //   }, 500);
  // }, []);

  return (
    <Block
      x={310}
      width={400}
      flexDir="col"
      fill="blue"
      //padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
      //textStyle={{ fill: 'red' }}
    >
      {Array.from({ length: 30 }).map((_, idx) => (
        <Block
          key={idx}
          width="100%"
          margin={{ top: idx === 0 ? 0 : 5 }}
          padding={{ left: 5, top: 5, right: 5, bottom: 5 }}
          fill="green"
        >
          <Block width={30} height={10} fill="red" />
          <Block width={30} height={10} fill="red" margin={{ left: 5 }} />
          <Block width={30} height={20} fill="red" margin={{ left: 5 }} />
          <Block width="grow" fill="yellow">
            <StaticTextBlock idx={0} text={`Hello ${idx}`} fontSize={15} />
            <StaticTextBlock idx={1} text="guys!" />
          </Block>
          <Block fill="yellow">
            <StaticTextBlock idx={0} text="Hello " />
            <StaticTextBlock idx={1} text="guys!" />
          </Block>
        </Block>
      ))}
    </Block>
  );
};

export const Test2 = () => {
  return (
    <Block
      width={250}
      height={200}
      fill="blue"
      padding={{ left: 10, right: 10, top: 10, bottom: 10 }}
    >
      <TextBlock text="ke 100 hi looool 555 1 keeeek" fill="red" fontSize={12} />
      <Block width="grow" fill="green">
        <TextBlock text="100 2" fill="blue" />
      </Block>
      <TextBlock text="100" fill="red" />
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
