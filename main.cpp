#include <tuple>

struct O {}; // dead
struct X {}; // alive

template <typename T>        // базовый вид — неопределенная функция
constexpr bool is_alive(); 

template<>                   // специальный вид для типа О
constexpr bool is_alive<O>() 
{ return false; }

template<>                   // специальный вид для типа X
constexpr bool is_alive<X>() 
{ return true; }

using start = std::tuple<
                O, O, O, O, O,
                O, O, X, O, O,
                O, O, O, X, O,
                O, X, X, X, O,
                O, O, O, O, O
              >;
              
// game parameters
const int width = 5;
const int height = 5;
const int iterations = 20;

template <typename tuple, int N>
struct tuple_counter
{
    constexpr static int value = is_alive<typename tuple_element<N, tuple>::type>() +
                + tuple_counter<tuple, N-1>::value;
};

template <typename tuple>       // выход из рекурсии при указанном N = 0
struct tuple_counter<tuple, 0>  
{
    constexpr static int value = is_alive<typename tuple_element<0, tuple>::type>();
};

template <typename point, typename neighbors>
struct calc_next_point_state
{
    constexpr static int neighbor_cnt =
            tuple_counter<neighbors, tuple_size<neighbors>() - 1>::value;

    using type =
        typename conditional <
            is_alive<point>(),
            typename conditional <
                (neighbor_cnt > 3) || (neighbor_cnt < 2),
                O,
                X
            >::type,
            typename conditional <
                (neighbor_cnt == 3),
                X,
                O
            >::type
        >::type;
};

template <typename initial_state>
struct level
{
    template <int N> // определить тип конкретной клетки
    using point = typename tuple_element<N, initial_state>::type;

    template <int N> // (занимает много места) определение типов соседей клетки
    using neighbors = tuple< point< /*индекс соседа*/ >, ... >;

    template <int N> // определение следующего типа клетки
    using next_point_state = typename calc_next_point_state<point<N>, neighbors<N>>::type;
};

template <typename tuple_1, typename tuple_2>
struct my_tuple_cat
{
// какой тип вернула бы tuple_cat, если бы мы вызвали ее с такими tuple, будь они у нас? 
    using result = decltype( tuple_cat( declval<tuple_1>(), declval<tuple_2>()  ) );
};

template <typename field, int iter>
struct next_field_state
{
    template<int N>
    using point = level<field>::next_point_state<N>;

    using next_field = typename my_tuple_cat <
                                    tuple< point<point_count - iter> >,
                                    typename next_field_state<field, iter-1>::next_field
                                >::result;
};

template <typename field>
struct next_field_state<field, 1>
{
    template<int N>
    using point = level<field>::next_point_state<N>;

    using next_field = tuple< point<point_count - 1> >;
};

template <typename Type>
void print();

template<>
void print<O>()
{ cout << "O"; }

template<>
void print<X>()
{ cout << "X"; }

template <typename tuple, int N>
struct Printer {
    static void print_tuple()
    {
        Printer<tuple, N-1>::print_tuple();
        if( N % width == 0 ) cout << endl;
        print<typename tuple_element<N, tuple>::type>();
    }
};

template <typename tuple>
struct Printer<tuple, 0> {
    static void print_tuple()
    {
        print<typename tuple_element<0, tuple>::type>();
    }
};

template <typename field, int iters>
struct game_process
{
    static void print()
    {
        Printer< field, point_count - 1 >::print_tuple();
        cout << endl << endl;
        game_process< typename next_field_state<field, point_count>::next_field, iters-1 >::print();
    }
};

template <typename field>
struct game_process<field, 0>
{
    static void print()
    {
        Printer< field, point_count - 1 >::print_tuple();
        cout << endl;
    }
};

int main()
{
    game_process< start, iterations >::print();
    return 0;
}

