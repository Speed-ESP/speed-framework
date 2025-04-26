#ifndef INC_UTILS_FOR_EACH_H_
#define INC_UTILS_FOR_EACH_H_

#define EXPAND(x) x
#define DEF_AUX_NARGS(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31, x32, x33, x34, x35, x36, x37, x38, x39, x40, x41, x42, x43, x44, x45, x46, x47, x48, x49, x50, x51, x52, x53, x54, x55, x56, x57, x58, x59, x60, x61, x62, x63, x64, VAL, ...) VAL
#define NARGS(...) EXPAND(DEF_AUX_NARGS(__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// --------------------------------------------------
#define FE0_1(what, x) EXPAND(what(x, 0))
#define FE0_2(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_1(what, __VA_ARGS__))
#define FE0_3(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_2(what, __VA_ARGS__))
#define FE0_4(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_3(what, __VA_ARGS__))
#define FE0_5(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_4(what, __VA_ARGS__))
#define FE0_6(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_5(what, __VA_ARGS__))
#define FE0_7(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_6(what, __VA_ARGS__))
#define FE0_8(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_7(what, __VA_ARGS__))
#define FE0_9(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_8(what, __VA_ARGS__))
#define FE0_10(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_9(what, __VA_ARGS__))
#define FE0_11(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_10(what, __VA_ARGS__))
#define FE0_12(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_11(what, __VA_ARGS__))
#define FE0_13(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_12(what, __VA_ARGS__))
#define FE0_14(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_13(what, __VA_ARGS__))
#define FE0_15(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_14(what, __VA_ARGS__))
#define FE0_16(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_15(what, __VA_ARGS__))
#define FE0_17(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_16(what, __VA_ARGS__))
#define FE0_18(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_17(what, __VA_ARGS__))
#define FE0_19(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_18(what, __VA_ARGS__))
#define FE0_20(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_19(what, __VA_ARGS__))
#define FE0_21(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_20(what, __VA_ARGS__))
#define FE0_22(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_21(what, __VA_ARGS__))
#define FE0_23(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_22(what, __VA_ARGS__))
#define FE0_24(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_23(what, __VA_ARGS__))
#define FE0_25(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_24(what, __VA_ARGS__))
#define FE0_26(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_25(what, __VA_ARGS__))
#define FE0_27(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_26(what, __VA_ARGS__))
#define FE0_28(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_27(what, __VA_ARGS__))
#define FE0_29(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_28(what, __VA_ARGS__))
#define FE0_30(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_29(what, __VA_ARGS__))
#define FE0_31(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_30(what, __VA_ARGS__))
#define FE0_32(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_31(what, __VA_ARGS__))
#define FE0_33(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_32(what, __VA_ARGS__))
#define FE0_34(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_33(what, __VA_ARGS__))
#define FE0_35(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_34(what, __VA_ARGS__))
#define FE0_36(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_35(what, __VA_ARGS__))
#define FE0_37(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_36(what, __VA_ARGS__))
#define FE0_38(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_37(what, __VA_ARGS__))
#define FE0_39(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_38(what, __VA_ARGS__))
#define FE0_40(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_39(what, __VA_ARGS__))
#define FE0_41(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_40(what, __VA_ARGS__))
#define FE0_42(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_41(what, __VA_ARGS__))
#define FE0_43(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_42(what, __VA_ARGS__))
#define FE0_44(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_43(what, __VA_ARGS__))
#define FE0_45(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_44(what, __VA_ARGS__))
#define FE0_46(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_45(what, __VA_ARGS__))
#define FE0_47(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_46(what, __VA_ARGS__))
#define FE0_48(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_47(what, __VA_ARGS__))
#define FE0_49(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_48(what, __VA_ARGS__))
#define FE0_50(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_49(what, __VA_ARGS__))
#define FE0_51(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_50(what, __VA_ARGS__))
#define FE0_52(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_51(what, __VA_ARGS__))
#define FE0_53(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_52(what, __VA_ARGS__))
#define FE0_54(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_53(what, __VA_ARGS__))
#define FE0_55(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_54(what, __VA_ARGS__))
#define FE0_56(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_55(what, __VA_ARGS__))
#define FE0_57(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_56(what, __VA_ARGS__))
#define FE0_58(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_57(what, __VA_ARGS__))
#define FE0_59(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_58(what, __VA_ARGS__))
#define FE0_60(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_59(what, __VA_ARGS__))
#define FE0_61(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_60(what, __VA_ARGS__))
#define FE0_62(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_61(what, __VA_ARGS__))
#define FE0_63(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_62(what, __VA_ARGS__))
#define FE0_64(what, x, ...) EXPAND(what(x, NARGS(__VA_ARGS__)) FE0_63(what, __VA_ARGS__))

#define REPEAT0_(...) EXPAND(DEF_AUX_NARGS(__VA_ARGS__, FE0_64, FE0_63, FE0_62, FE0_61, FE0_60, FE0_59, FE0_58, FE0_57, FE0_56, FE0_55, FE0_54, FE0_53, FE0_52, FE0_51, FE0_50, FE0_49, FE0_48, FE0_47, FE0_46, FE0_45, FE0_44, FE0_43, FE0_42, FE0_41, FE0_40, FE0_39, FE0_38, FE0_37, FE0_36, FE0_35, FE0_34, FE0_33, FE0_32, FE0_31, FE0_30, FE0_29, FE0_28, FE0_27, FE0_26, FE0_25, FE0_24, FE0_23, FE0_22, FE0_21, FE0_20, FE0_19, FE0_18, FE0_17, FE0_16, FE0_15, FE0_14, FE0_13, FE0_12, FE0_11, FE0_10, FE0_9, FE0_8, FE0_7, FE0_6, FE0_5, FE0_4, FE0_3, FE0_2, FE0_1, 0))
#define FOR_EACH(what, ...) EXPAND(REPEAT0_(__VA_ARGS__)(what, __VA_ARGS__))

// --------------------------------------------------
#define FE1_1(what, x, y) EXPAND(what(x, y, 0))
#define FE1_2(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_1(what, x, __VA_ARGS__))
#define FE1_3(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_2(what, x, __VA_ARGS__))
#define FE1_4(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_3(what, x, __VA_ARGS__))
#define FE1_5(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_4(what, x, __VA_ARGS__))
#define FE1_6(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_5(what, x, __VA_ARGS__))
#define FE1_7(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_6(what, x, __VA_ARGS__))
#define FE1_8(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_7(what, x, __VA_ARGS__))
#define FE1_9(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_8(what, x, y, __VA_ARGS__))
#define FE1_10(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_9(what, x, y, __VA_ARGS__))
#define FE1_11(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_10(what, x, y, __VA_ARGS__))
#define FE1_12(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_11(what, x, y, __VA_ARGS__))
#define FE1_13(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_12(what, x, y, __VA_ARGS__))
#define FE1_14(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_13(what, x, y, __VA_ARGS__))
#define FE1_15(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_14(what, x, y, __VA_ARGS__))
#define FE1_16(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_15(what, x, y, __VA_ARGS__))
#define FE1_17(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_16(what, x, y, __VA_ARGS__))
#define FE1_18(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_17(what, x, y, __VA_ARGS__))
#define FE1_19(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_18(what, x, y, __VA_ARGS__))
#define FE1_20(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_19(what, x, y, __VA_ARGS__))
#define FE1_21(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_20(what, x, y, __VA_ARGS__))
#define FE1_22(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_21(what, x, y, __VA_ARGS__))
#define FE1_23(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_22(what, x, y, __VA_ARGS__))
#define FE1_24(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_23(what, x, y, __VA_ARGS__))
#define FE1_25(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_24(what, x, y, __VA_ARGS__))
#define FE1_26(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_25(what, x, y, __VA_ARGS__))
#define FE1_27(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_26(what, x, y, __VA_ARGS__))
#define FE1_28(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_27(what, x, y, __VA_ARGS__))
#define FE1_29(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_28(what, x, y, __VA_ARGS__))
#define FE1_30(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_29(what, x, y, __VA_ARGS__))
#define FE1_31(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_30(what, x, y, __VA_ARGS__))
#define FE1_32(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_31(what, x, y, __VA_ARGS__))
#define FE1_33(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_32(what, x, y, __VA_ARGS__))
#define FE1_34(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_33(what, x, y, __VA_ARGS__))
#define FE1_35(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_34(what, x, y, __VA_ARGS__))
#define FE1_36(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_35(what, x, y, __VA_ARGS__))
#define FE1_37(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_36(what, x, y, __VA_ARGS__))
#define FE1_38(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_37(what, x, y, __VA_ARGS__))
#define FE1_39(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_38(what, x, y, __VA_ARGS__))
#define FE1_40(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_39(what, x, y, __VA_ARGS__))
#define FE1_41(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_40(what, x, y, __VA_ARGS__))
#define FE1_42(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_41(what, x, y, __VA_ARGS__))
#define FE1_43(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_42(what, x, y, __VA_ARGS__))
#define FE1_44(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_43(what, x, y, __VA_ARGS__))
#define FE1_45(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_44(what, x, y, __VA_ARGS__))
#define FE1_46(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_45(what, x, y, __VA_ARGS__))
#define FE1_47(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_46(what, x, y, __VA_ARGS__))
#define FE1_48(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_47(what, x, y, __VA_ARGS__))
#define FE1_49(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_48(what, x, y, __VA_ARGS__))
#define FE1_50(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_49(what, x, y, __VA_ARGS__))
#define FE1_51(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_50(what, x, y, __VA_ARGS__))
#define FE1_52(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_51(what, x, y, __VA_ARGS__))
#define FE1_53(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_52(what, x, y, __VA_ARGS__))
#define FE1_54(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_53(what, x, y, __VA_ARGS__))
#define FE1_55(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_54(what, x, y, __VA_ARGS__))
#define FE1_56(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_55(what, x, y, __VA_ARGS__))
#define FE1_57(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_56(what, x, y, __VA_ARGS__))
#define FE1_58(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_57(what, x, y, __VA_ARGS__))
#define FE1_59(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_58(what, x, y, __VA_ARGS__))
#define FE1_60(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_59(what, x, y, __VA_ARGS__))
#define FE1_61(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_60(what, x, y, __VA_ARGS__))
#define FE1_62(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_61(what, x, y, __VA_ARGS__))
#define FE1_63(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_62(what, x, y, __VA_ARGS__))
#define FE1_64(what, x, y, ...) EXPAND(what(x, y, NARGS(__VA_ARGS__)) FE1_63(what, x, y, __VA_ARGS__))

#define REPEAT1_(...) EXPAND(DEF_AUX_NARGS(__VA_ARGS__, FE1_64, FE1_63, FE1_62, FE1_61, FE1_60, FE1_59, FE1_58, FE1_57, FE1_56, FE1_55, FE1_54, FE1_53, FE1_52, FE1_51, FE1_50, FE1_49, FE1_48, FE1_47, FE1_46, FE1_45, FE1_44, FE1_43, FE1_42, FE1_41, FE1_40, FE1_39, FE1_38, FE1_37, FE1_36, FE1_35, FE1_34, FE1_33, FE1_32, FE1_31, FE1_30, FE1_29, FE1_28, FE1_27, FE1_26, FE1_25, FE1_24, FE1_23, FE1_22, FE1_21, FE1_20, FE1_19, FE1_18, FE1_17, FE1_16, FE1_15, FE1_14, FE1_13, FE1_12, FE1_11, FE1_10, FE1_9, FE1_8, FE1_7, FE1_6, FE1_5, FE1_4, FE1_3, FE1_2, FE1_1, 0))
#define FOR_EACH_PIVOT_1ST_ARG(what, arg0, ...) EXPAND(REPEAT1_(__VA_ARGS__)(what, arg0, __VA_ARGS__))

// --------------------------------------------------
#define APPLY1(FN, x, y) EXPAND(FN(x, y))
#define APPLY2(FN, x, y, ...) EXPAND(FN(x, y) APPLY1(FN, __VA_ARGS__))
#define APPLY3(FN, x, y, ...) EXPAND(FN(x, y) APPLY2(FN, __VA_ARGS__))
#define APPLY4(FN, x, y, ...) EXPAND(FN(x, y) APPLY3(FN, __VA_ARGS__))
#define APPLY5(FN, x, y, ...) EXPAND(FN(x, y) APPLY4(FN, __VA_ARGS__))
#define APPLY6(FN, x, y, ...) EXPAND(FN(x, y) APPLY5(FN, __VA_ARGS__))
#define APPLY7(FN, x, y, ...) EXPAND(FN(x, y) APPLY6(FN, __VA_ARGS__))
#define APPLY8(FN, x, y, ...) EXPAND(FN(x, y) APPLY7(FN, __VA_ARGS__))
#define APPLY9(FN, x, y, ...) EXPAND(FN(x, y) APPLY8(FN, __VA_ARGS__))
#define APPLY10(FN, x, y, ...) EXPAND(FN(x, y) APPLY9(FN, __VA_ARGS__))
#define APPLY11(FN, x, y, ...) EXPAND(FN(x, y) APPLY10(FN, __VA_ARGS__))
#define APPLY12(FN, x, y, ...) EXPAND(FN(x, y) APPLY11(FN, __VA_ARGS__))
#define APPLY13(FN, x, y, ...) EXPAND(FN(x, y) APPLY12(FN, __VA_ARGS__))
#define APPLY14(FN, x, y, ...) EXPAND(FN(x, y) APPLY13(FN, __VA_ARGS__))
#define APPLY15(FN, x, y, ...) EXPAND(FN(x, y) APPLY14(FN, __VA_ARGS__))
#define APPLY16(FN, x, y, ...) EXPAND(FN(x, y) APPLY15(FN, __VA_ARGS__))
#define APPLY17(FN, x, y, ...) EXPAND(FN(x, y) APPLY16(FN, __VA_ARGS__))
#define APPLY18(FN, x, y, ...) EXPAND(FN(x, y) APPLY17(FN, __VA_ARGS__))
#define APPLY19(FN, x, y, ...) EXPAND(FN(x, y) APPLY18(FN, __VA_ARGS__))
#define APPLY20(FN, x, y, ...) EXPAND(FN(x, y) APPLY19(FN, __VA_ARGS__))
#define APPLY21(FN, x, y, ...) EXPAND(FN(x, y) APPLY20(FN, __VA_ARGS__))
#define APPLY22(FN, x, y, ...) EXPAND(FN(x, y) APPLY21(FN, __VA_ARGS__))
#define APPLY23(FN, x, y, ...) EXPAND(FN(x, y) APPLY22(FN, __VA_ARGS__))
#define APPLY24(FN, x, y, ...) EXPAND(FN(x, y) APPLY23(FN, __VA_ARGS__))
#define APPLY25(FN, x, y, ...) EXPAND(FN(x, y) APPLY24(FN, __VA_ARGS__))
#define APPLY26(FN, x, y, ...) EXPAND(FN(x, y) APPLY25(FN, __VA_ARGS__))
#define APPLY27(FN, x, y, ...) EXPAND(FN(x, y) APPLY26(FN, __VA_ARGS__))
#define APPLY28(FN, x, y, ...) EXPAND(FN(x, y) APPLY27(FN, __VA_ARGS__))
#define APPLY29(FN, x, y, ...) EXPAND(FN(x, y) APPLY28(FN, __VA_ARGS__))
#define APPLY30(FN, x, y, ...) EXPAND(FN(x, y) APPLY29(FN, __VA_ARGS__))
#define APPLY31(FN, x, y, ...) EXPAND(FN(x, y) APPLY30(FN, __VA_ARGS__))
#define APPLY32(FN, x, y, ...) EXPAND(FN(x, y) APPLY31(FN, __VA_ARGS__))

// --------------------------------------------------

#define APPLY_ARG1_1(FN, x, y, z) EXPAND(FN(x, y, z))
#define APPLY_ARG1_2(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_1(FN, x, __VA_ARGS__))
#define APPLY_ARG1_3(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_2(FN, x, __VA_ARGS__))
#define APPLY_ARG1_4(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_3(FN, x, __VA_ARGS__))
#define APPLY_ARG1_5(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_4(FN, x, __VA_ARGS__))
#define APPLY_ARG1_6(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_5(FN, x, __VA_ARGS__))
#define APPLY_ARG1_7(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_6(FN, x, __VA_ARGS__))
#define APPLY_ARG1_8(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_7(FN, x, __VA_ARGS__))
#define APPLY_ARG1_9(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_8(FN, x, __VA_ARGS__))
#define APPLY_ARG1_10(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_9(FN, x, __VA_ARGS__))
#define APPLY_ARG1_11(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_10(FN, x, __VA_ARGS__))
#define APPLY_ARG1_12(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_11(FN, x, __VA_ARGS__))
#define APPLY_ARG1_13(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_12(FN, x, __VA_ARGS__))
#define APPLY_ARG1_14(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_13(FN, x, __VA_ARGS__))
#define APPLY_ARG1_15(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_14(FN, x, __VA_ARGS__))
#define APPLY_ARG1_16(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_15(FN, x, __VA_ARGS__))
#define APPLY_ARG1_17(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_16(FN, x, __VA_ARGS__))
#define APPLY_ARG1_18(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_17(FN, x, __VA_ARGS__))
#define APPLY_ARG1_19(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_18(FN, x, __VA_ARGS__))
#define APPLY_ARG1_20(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_19(FN, x, __VA_ARGS__))
#define APPLY_ARG1_21(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_20(FN, x, __VA_ARGS__))
#define APPLY_ARG1_22(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_21(FN, x, __VA_ARGS__))
#define APPLY_ARG1_23(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_22(FN, x, __VA_ARGS__))
#define APPLY_ARG1_24(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_23(FN, x, __VA_ARGS__))
#define APPLY_ARG1_25(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_24(FN, x, __VA_ARGS__))
#define APPLY_ARG1_26(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_25(FN, x, __VA_ARGS__))
#define APPLY_ARG1_27(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_26(FN, x, __VA_ARGS__))
#define APPLY_ARG1_28(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_27(FN, x, __VA_ARGS__))
#define APPLY_ARG1_29(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_28(FN, x, __VA_ARGS__))
#define APPLY_ARG1_30(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_29(FN, x, __VA_ARGS__))
#define APPLY_ARG1_31(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_30(FN, x, __VA_ARGS__))
#define APPLY_ARG1_32(FN, x, y, z, ...) EXPAND(FN(x, y, z) APPLY_ARG1_31(FN, x, __VA_ARGS__))

#define NPAIRARGS(...)                                                            \
    DEF_AUX_NARGS(__VA_ARGS__,                                                    \
                  64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
                  48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, \
                  32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
                  16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define NPAIRARGS_IMPL(_64, _63, _62, _61, _60, _59, _58, _57, _56, _55, _54, _53, _52, _51, _50, _49, \
                       _48, _47, _46, _45, _44, _43, _42, _41, _40, _39, _38, _37, _36, _35, _34, _33, \
                       _32, _31, _30, _29, _28, _27, _26, _25, _24, _23, _22, _21, _20, _19, _18, _17, \
                       _16, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N

#define APPLYNPAIRARGS(...) EXPAND(DEF_AUX_NARGS(__VA_ARGS__,                                                            \
                                                 APPLY32, APPLY32, APPLY31, APPLY31, APPLY30, APPLY30, APPLY29, APPLY29, \
                                                 APPLY28, APPLY28, APPLY27, APPLY27, APPLY26, APPLY26, APPLY25, APPLY25, \
                                                 APPLY24, APPLY24, APPLY23, APPLY23, APPLY22, APPLY22, APPLY21, APPLY21, \
                                                 APPLY20, APPLY20, APPLY19, APPLY19, APPLY18, APPLY18, APPLY17, APPLY17, \
                                                 APPLY16, APPLY16, APPLY15, APPLY15, APPLY14, APPLY14, APPLY13, APPLY13, \
                                                 APPLY12, APPLY12, APPLY11, APPLY11, APPLY10, APPLY10, APPLY9, APPLY9,   \
                                                 APPLY8, APPLY8, APPLY7, APPLY7, APPLY6, APPLY6, APPLY5, APPLY5, APPLY4, \
                                                 APPLY4, APPLY3, APPLY3, APPLY2, APPLY2, APPLY1, APPLY1))

#define FOR_EACH_PAIR(FN, ...) EXPAND(APPLYNPAIRARGS(__VA_ARGS__)(FN, __VA_ARGS__))

#define APPLYNPAIRARGS1(...) EXPAND(DEF_AUX_NARGS(__VA_ARGS__,                                                                                                            \
                                                  APPLY_ARG1_32, APPLY_ARG1_32, APPLY_ARG1_31, APPLY_ARG1_31, APPLY_ARG1_30, APPLY_ARG1_30, APPLY_ARG1_29, APPLY_ARG1_29, \
                                                  APPLY_ARG1_28, APPLY_ARG1_28, APPLY_ARG1_27, APPLY_ARG1_27, APPLY_ARG1_26, APPLY_ARG1_26, APPLY_ARG1_25, APPLY_ARG1_25, \
                                                  APPLY_ARG1_24, APPLY_ARG1_24, APPLY_ARG1_23, APPLY_ARG1_23, APPLY_ARG1_22, APPLY_ARG1_22, APPLY_ARG1_21, APPLY_ARG1_21, \
                                                  APPLY_ARG1_20, APPLY_ARG1_20, APPLY_ARG1_19, APPLY_ARG1_19, APPLY_ARG1_18, APPLY_ARG1_18, APPLY_ARG1_17, APPLY_ARG1_17, \
                                                  APPLY_ARG1_16, APPLY_ARG1_16, APPLY_ARG1_15, APPLY_ARG1_15, APPLY_ARG1_14, APPLY_ARG1_14, APPLY_ARG1_13, APPLY_ARG1_13, \
                                                  APPLY_ARG1_12, APPLY_ARG1_12, APPLY_ARG1_11, APPLY_ARG1_11, APPLY_ARG1_10, APPLY_ARG1_10, APPLY_ARG1_9, APPLY_ARG1_9,   \
                                                  APPLY_ARG1_8, APPLY_ARG1_8, APPLY_ARG1_7, APPLY_ARG1_7, APPLY_ARG1_6, APPLY_ARG1_6, APPLY_ARG1_5, APPLY_ARG1_5,         \
                                                  APPLY_ARG1_4, APPLY_ARG1_4, APPLY_ARG1_3, APPLY_ARG1_3, APPLY_ARG1_2, APPLY_ARG1_2, APPLY_ARG1_1, APPLY_ARG1_1))

#define FOR_EACH_PAIR_PIVOT_1ST_ARG(FN, arg0, ...) EXPAND(APPLYNPAIRARGS1(__VA_ARGS__)(FN, arg0, __VA_ARGS__))
// --------------------------------------------------

#endif